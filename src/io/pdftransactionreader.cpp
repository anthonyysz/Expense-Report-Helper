#include "pdftransactionreader.h"

#include <QFileInfo>
#include <QHash>
#include <QPdfDocument>
#include <QPdfSelection>
#include <QRegularExpression>

#include <algorithm>

QVector<Transaction> PdfTransactionReader::readTransactions(const QStringList& filePaths, QStringList* errors) const
{
	QVector<Transaction> transactions;

	for (const QString& filePath : filePaths) {
		transactions.append(readFile(filePath, errors));
	}

	return transactions;
}

QVector<Transaction> PdfTransactionReader::readFile(const QString& filePath, QStringList* errors) const
{
	QPdfDocument document;
	const QPdfDocument::Error error = document.load(filePath);

	if (error != QPdfDocument::Error::None) {
		if (errors != nullptr) {
			errors->append(QFileInfo(filePath).fileName() + ": could not be opened as a selectable-text PDF.");
		}
		return {};
	}

	QString text;

	for (int page = 0; page < document.pageCount(); ++page) {
		const QPdfSelection selection = document.getAllText(page);
		if (selection.isValid()) {
			text += selection.text();
			text += '\n';
		}
	}

	QVector<Transaction> transactions = parseText(filePath, text);

	if (transactions.isEmpty() && errors != nullptr) {
		errors->append(QFileInfo(filePath).fileName() + ": no transactions were recognized.");
	}

	return transactions;
}

QVector<Transaction> PdfTransactionReader::parseText(const QString& filePath, const QString& text) const
{
	QVector<Transaction> bankOfAmericaTransactions = parseBankOfAmericaText(filePath, text);

	if (!bankOfAmericaTransactions.isEmpty()) {
		return bankOfAmericaTransactions;
	}

	QVector<Transaction> capitalOneTransactions = parseCapitalOneText(filePath, text);

	if (!capitalOneTransactions.isEmpty()) {
		return capitalOneTransactions;
	}

	QVector<Transaction> transactions;
	const QString sourceName = QFileInfo(filePath).fileName();
	const QStringList lines = text.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
	const QRegularExpression transactionLine(
		R"(^\s*(\d{1,2}[\/\-]\d{1,2}(?:[\/\-]\d{2,4})?)\s+(.+?)\s+(-?\(?\$?\d{1,3}(?:,\d{3})*(?:\.\d{2})\)?)\s*$)");
	const QRegularExpression skipWords(
		R"(\b(balance|payment due|minimum payment|account number|credit limit|available credit|interest|fees charged|total)\b)",
		QRegularExpression::CaseInsensitiveOption);
	int inferredYear = QDate::currentDate().year();

	for (const QString& rawLine : lines) {
		const QString line = rawLine.simplified();

		if (line.contains(skipWords)) {
			continue;
		}

		const QRegularExpressionMatch match = transactionLine.match(line);

		if (!match.hasMatch()) {
			continue;
		}

		QString dateText = match.captured(1);
		const QString description = match.captured(2).trimmed();
		QString amountText = match.captured(3);

		if (description.size() < 2) {
			continue;
		}

		QDate date;

		for (const QString& format : { QString("M/d/yyyy"), QString("MM/dd/yyyy"), QString("M-d-yyyy"), QString("MM-dd-yyyy"), QString("M/d/yy"), QString("MM/dd/yy"), QString("M-d-yy"), QString("MM-dd-yy") }) {
			date = QDate::fromString(dateText, format);
			if (date.isValid()) {
				break;
			}
		}

		if (!date.isValid()) {
			date = QDate::fromString(dateText + "/" + QString::number(inferredYear), "M/d/yyyy");
		}

		if (!date.isValid()) {
			continue;
		}

		const bool negativeByParentheses = amountText.startsWith('(') && amountText.endsWith(')');
		amountText.remove('$');
		amountText.remove(',');
		amountText.remove('(');
		amountText.remove(')');

		bool ok = false;
		double amount = amountText.toDouble(&ok);

		if (!ok) {
			continue;
		}

		if (negativeByParentheses) {
			amount = -amount;
		}

		Transaction transaction;
		transaction.sourceFile = sourceName;
		transaction.date = date;
		transaction.description = description;
		transaction.amount = amount;
		transactions.append(transaction);
	}

	std::stable_sort(transactions.begin(), transactions.end(), [](const Transaction& left, const Transaction& right) {
		return left.date < right.date;
	});

	return transactions;
}

QVector<Transaction> PdfTransactionReader::parseBankOfAmericaText(const QString& filePath, const QString& text) const
{
	QVector<Transaction> transactions;
	const QString sourceName = QFileInfo(filePath).fileName();
	const QStringList lines = text.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
	const QRegularExpression dateStart(R"(^\s*\d{2}/\d{2}/\d{4}\b)");
	const QRegularExpression amountEnd(R"(-?\$?\d{1,3}(?:,\d{3})*(?:\.\d{2})\s*$)");
	QString currentRecord;

	for (const QString& rawLine : lines) {
		const QString line = rawLine.simplified();

		if (line.isEmpty()
			|| line == "Transactions"
			|| line == "Posting date Description Type Amount"
			|| line.startsWith("Balance Summary:")
			|| line.startsWith("View:")
			|| line.startsWith("Showing results")
			|| line.contains("Bank of America | Online Banking")
			|| line.startsWith("https://")) {
			continue;
		}

		if (dateStart.match(line).hasMatch()) {
			if (!currentRecord.isEmpty()) {
				appendBankOfAmericaRecord(transactions, sourceName, currentRecord);
			}

			currentRecord = line;
		}
		else if (!currentRecord.isEmpty()) {
			currentRecord += " " + line;
		}

		if (!currentRecord.isEmpty() && amountEnd.match(currentRecord).hasMatch()) {
			appendBankOfAmericaRecord(transactions, sourceName, currentRecord);
			currentRecord.clear();
		}
	}

	if (!currentRecord.isEmpty()) {
		appendBankOfAmericaRecord(transactions, sourceName, currentRecord);
	}

	std::stable_sort(transactions.begin(), transactions.end(), [](const Transaction& left, const Transaction& right) {
		return left.date < right.date;
	});

	return transactions;
}

bool PdfTransactionReader::appendBankOfAmericaRecord(QVector<Transaction>& transactions, const QString& sourceName, const QString& record) const
{
	static const QRegularExpression pattern(
		R"(^\s*(\d{2}/\d{2}/\d{4})\s+(.+?)\s+(-?\$?\d{1,3}(?:,\d{3})*(?:\.\d{2}))\s*$)");
	static const QRegularExpression typeTail(
		R"(\s+(?:Other\s+)?(?:Payment|Deposit|Transfer|Debit Card|Virtual Card)\s*$)",
		QRegularExpression::CaseInsensitiveOption);

	const QRegularExpressionMatch match = pattern.match(record.simplified());

	if (!match.hasMatch()) {
		return false;
	}

	const QDate date = QDate::fromString(match.captured(1), "MM/dd/yyyy");

	if (!date.isValid()) {
		return false;
	}

	QString description = match.captured(2).trimmed();
	description.replace(typeTail, "");
	description.replace(QRegularExpression(R"(\s+)"), " ");
	description = description.trimmed();

	QString amountText = match.captured(3);
	amountText.remove('$');
	amountText.remove(',');

	bool ok = false;
	const double amount = amountText.toDouble(&ok);

	if (!ok) {
		return false;
	}

	Transaction transaction;
	transaction.sourceFile = sourceName;
	transaction.date = date;
	transaction.description = description;
	transaction.amount = amount;
	transactions.append(transaction);
	return true;
}

QVector<Transaction> PdfTransactionReader::parseCapitalOneText(const QString& filePath, const QString& text) const
{
	QVector<Transaction> transactions;
	const QString sourceName = QFileInfo(filePath).fileName();
	const QStringList lines = text.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
	const QRegularExpression dateStart(R"(^\s*(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\s+\d{1,2}\b)", QRegularExpression::CaseInsensitiveOption);
	const QRegularExpression amountLine(R"(^\s*-?\$?\d{1,3}(?:,\d{3})*(?:\.\d{2})\s*$)");
	const QRegularExpression filterYear(R"(Filter By:\s*\d{2}/\d{2}/(\d{4}))");
	const QRegularExpressionMatch yearMatch = filterYear.match(text);
	const int year = yearMatch.hasMatch() ? yearMatch.captured(1).toInt() : QDate::currentDate().year();
	QString currentRecord;

	for (const QString& rawLine : lines) {
		QString line = rawLine.simplified();
		line.replace(QChar(0xFB01), "fi");

		if (line.isEmpty()
			|| line.startsWith("Filter By:")
			|| line == "DATE DESCRIPTION CATEGORY CARD AMOUNT"
			|| line.contains("Capital One")
			|| line.startsWith("https://")) {
			continue;
		}

		if (dateStart.match(line).hasMatch()) {
			if (!currentRecord.isEmpty()) {
				appendCapitalOneRecord(transactions, sourceName, currentRecord, year);
			}

			currentRecord = line;
		}
		else if (!currentRecord.isEmpty()) {
			currentRecord += " " + line;
		}

		if (!currentRecord.isEmpty() && amountLine.match(line).hasMatch()) {
			appendCapitalOneRecord(transactions, sourceName, currentRecord, year);
			currentRecord.clear();
		}
	}

	if (!currentRecord.isEmpty()) {
		appendCapitalOneRecord(transactions, sourceName, currentRecord, year);
	}

	std::stable_sort(transactions.begin(), transactions.end(), [](const Transaction& left, const Transaction& right) {
		return left.date < right.date;
	});

	return transactions;
}

bool PdfTransactionReader::appendCapitalOneRecord(QVector<Transaction>& transactions, const QString& sourceName, const QString& record, int year) const
{
	static const QRegularExpression pattern(
		R"(^\s*(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\s+(\d{1,2})\s+(.+?)\s+(-?\$?\d{1,3}(?:,\d{3})*(?:\.\d{2}))\s*$)",
		QRegularExpression::CaseInsensitiveOption);
	static const QStringList categories = {
		"Gas/Automotive",
		"Other Travel",
		"Phone/Cable",
		"Healthcare",
		"Merchandise",
		"Insurance",
		"Utilities",
		"Grocery",
		"Payment",
		"Dining",
		"Other"
	};

	const QRegularExpressionMatch match = pattern.match(record.simplified());

	if (!match.hasMatch()) {
		return false;
	}

	const QString monthText = match.captured(1).left(3).toLower();
	const int day = match.captured(2).toInt();
	const QHash<QString, int> monthByName = {
		{ "jan", 1 },
		{ "feb", 2 },
		{ "mar", 3 },
		{ "apr", 4 },
		{ "may", 5 },
		{ "jun", 6 },
		{ "jul", 7 },
		{ "aug", 8 },
		{ "sep", 9 },
		{ "oct", 10 },
		{ "nov", 11 },
		{ "dec", 12 }
	};
	const QDate date(year, monthByName.value(monthText, 0), day);

	if (!date.isValid()) {
		return false;
	}

	QString descriptionAndMeta = match.captured(3).trimmed();
	descriptionAndMeta.replace(QChar(0xFB01), "fi");
	descriptionAndMeta.replace(QRegularExpression(R"(\s+)"), " ");

	int categoryIndex = -1;

	for (const QString& category : categories) {
		const int index = descriptionAndMeta.indexOf(" " + category + " Anthony S.", 0, Qt::CaseInsensitive);

		if (index >= 0 && (categoryIndex < 0 || index < categoryIndex)) {
			categoryIndex = index;
		}
	}

	QString description = categoryIndex >= 0 ? descriptionAndMeta.left(categoryIndex).trimmed() : descriptionAndMeta;
	description.replace(QRegularExpression(R"(\s*-\s*)"), " - ");
	description = description.trimmed();

	QString amountText = match.captured(4);
	amountText.remove('$');
	amountText.remove(',');

	bool ok = false;
	const double amount = amountText.toDouble(&ok);

	if (!ok || description.isEmpty()) {
		return false;
	}

	Transaction transaction;
	transaction.sourceFile = sourceName;
	transaction.date = date;
	transaction.description = description;
	transaction.amount = amount;
	transactions.append(transaction);
	return true;
}
