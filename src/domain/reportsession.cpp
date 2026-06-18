#include "reportsession.h"

#include <algorithm>

void ReportSession::setTransactions(const QVector<Transaction>& transactions)
{
	reportTransactions = transactions;
	transactionIndex = 0;
	assignmentHistory.clear();
}

bool ReportSession::hasTransactions() const
{
	return !reportTransactions.isEmpty();
}

bool ReportSession::isComplete() const
{
	return hasTransactions() && transactionIndex >= reportTransactions.size();
}

int ReportSession::currentIndex() const
{
	return transactionIndex;
}

int ReportSession::totalCount() const
{
	return reportTransactions.size();
}

const Transaction* ReportSession::currentTransaction() const
{
	if (transactionIndex < 0 || transactionIndex >= reportTransactions.size()) {
		return nullptr;
	}

	return &reportTransactions[transactionIndex];
}

const QVector<Transaction>& ReportSession::transactions() const
{
	return reportTransactions;
}

void ReportSession::assignCurrent(const QString& sectionPath)
{
	if (transactionIndex < 0 || transactionIndex >= reportTransactions.size()) {
		return;
	}

	reportTransactions[transactionIndex].assignedPath = sectionPath;
	reportTransactions[transactionIndex].skipped = false;
	assignmentHistory.append(transactionIndex);
	++transactionIndex;
}

void ReportSession::skipCurrent()
{
	if (transactionIndex < 0 || transactionIndex >= reportTransactions.size()) {
		return;
	}

	reportTransactions[transactionIndex].assignedPath.clear();
	reportTransactions[transactionIndex].skipped = true;
	assignmentHistory.append(transactionIndex);
	++transactionIndex;
}

void ReportSession::addManualTransaction(const Transaction& transaction)
{
	reportTransactions.append(transaction);

	if (transactionIndex > reportTransactions.size()) {
		transactionIndex = reportTransactions.size();
	}
}

bool ReportSession::undo()
{
	if (assignmentHistory.isEmpty()) {
		return false;
	}

	transactionIndex = assignmentHistory.takeLast();
	reportTransactions[transactionIndex].assignedPath.clear();
	reportTransactions[transactionIndex].skipped = false;
	return true;
}

QVector<ReportSummaryRow> ReportSession::summaryRows(const BudgetFormat& format) const
{
	QVector<ReportSummaryRow> rows;
	QHash<QString, int> rowByPath;

	for (const BudgetSection& root : format.roots()) {
		for (const QString& path : leafPaths(root, QString())) {
			ReportSummaryRow row;
			row.path = path;
			rowByPath.insert(path, rows.size());
			rows.append(row);
		}
	}

	for (const Transaction& transaction : reportTransactions) {
		if (transaction.assignedPath.isEmpty() || transaction.skipped) {
			continue;
		}

		if (!rowByPath.contains(transaction.assignedPath)) {
			ReportSummaryRow row;
			row.path = transaction.assignedPath;
			rowByPath.insert(row.path, rows.size());
			rows.append(row);
		}

		ReportSummaryRow& row = rows[rowByPath.value(transaction.assignedPath)];
		row.total += std::abs(transaction.amount);
		row.transactions.append(transaction);
	}

	return rows;
}

QVector<QString> ReportSession::leafPaths(const BudgetSection& section, const QString& prefix) const
{
	const QString path = prefix.isEmpty() ? section.name : prefix + " / " + section.name;

	if (section.children.isEmpty()) {
		return { path };
	}

	QVector<QString> paths;

	for (const BudgetSection& child : section.children) {
		paths.append(leafPaths(child, path));
	}

	return paths;
}
