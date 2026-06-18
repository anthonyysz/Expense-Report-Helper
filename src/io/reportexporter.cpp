#include "reportexporter.h"

#include <QDateTime>
#include <QFile>
#include <QFont>
#include <QColor>
#include <QHash>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QRegularExpression>
#include <QStringList>
#include <QTextOption>

#include <cmath>

bool ReportExporter::exportPdf(const QString& filePath, const ReportSession& session, const BudgetFormat& format, QString* error) const
{
	QPdfWriter writer(filePath);
	writer.setPageSize(QPageSize(QPageSize::Letter));
	writer.setResolution(144);

	QPainter painter(&writer);

	if (!painter.isActive()) {
		if (error != nullptr) {
			*error = "Could not create PDF file.";
		}
		return false;
	}

	const int margin = 72;
	const int bottom = writer.height() - margin;
	int y = margin;

	auto newPageIfNeeded = [&](int neededHeight) {
		if (y + neededHeight > bottom) {
			writer.newPage();
			y = margin;
		}
	};

	QFont titleFont = painter.font();
	titleFont.setPointSize(22);
	titleFont.setBold(true);
	painter.setFont(titleFont);
	painter.drawText(margin, y, "ExpenseBot Report");
	y += 44;

	QFont normalFont = painter.font();
	normalFont.setPointSize(10);
	normalFont.setBold(false);
	painter.setFont(normalFont);
	painter.drawText(margin, y, "Generated " + QDateTime::currentDateTime().toString("yyyy-MM-dd h:mm AP"));
	y += 42;

	const QVector<SummaryItem> items = summaryItems(session, format);
	const QVector<SectionSummary> incomeSections = sectionSummaries(items, "Income");
	const QVector<SectionSummary> expenseSections = sectionSummaries(items, "Expenses");
	double incomeTotal = 0.0;
	double expenseTotal = 0.0;

	for (const SummaryItem& item : items) {
		if (item.root == "Income") {
			incomeTotal += item.total;
		}
		else if (item.root == "Expenses") {
			expenseTotal += item.total;
		}
	}

	QFont headingFont = normalFont;
	headingFont.setPointSize(13);
	headingFont.setBold(true);

	QFont smallFont = normalFont;
	smallFont.setPointSize(9);

	const int pageWidth = writer.width() - margin * 2;
	const int amountX = margin + pageWidth - 150;

	auto drawTotalCard = [&](int x, const QString& label, double value, const QColor& fill) {
		const int cardWidth = (pageWidth - 24) / 3;
		painter.fillRect(QRect(x, y, cardWidth, 64), fill);
		painter.setPen(QColor(70, 70, 70));
		painter.setFont(smallFont);
		painter.drawText(x + 14, y + 22, label);
		painter.setFont(headingFont);
		painter.drawText(QRect(x + 14, y + 30, cardWidth - 28, 26), Qt::AlignRight | Qt::AlignVCenter, money(value));
		painter.setPen(Qt::black);
	};

	drawTotalCard(margin, "Total Money In", incomeTotal, QColor(232, 245, 236));
	drawTotalCard(margin + ((pageWidth - 24) / 3) + 12, "Total Expenses", expenseTotal, QColor(252, 235, 235));
	drawTotalCard(margin + (((pageWidth - 24) / 3) + 12) * 2, "Net", incomeTotal - expenseTotal, QColor(238, 242, 252));
	y += 92;

	auto drawSectionGroup = [&](const QString& title, const QVector<SectionSummary>& sections, const QColor& titleFill) {
		newPageIfNeeded(54);
		painter.fillRect(QRect(margin, y, pageWidth, 34), titleFill);
		painter.setFont(headingFont);
		painter.drawText(margin + 12, y + 23, title);
		y += 48;

		for (const SectionSummary& section : sections) {
			if (section.total == 0.0) {
				continue;
			}

			const int sectionHeight = 34 + qMax(1, section.tags.size()) * 24 + 18;
			newPageIfNeeded(sectionHeight);

			painter.setFont(headingFont);
			painter.drawText(margin + 10, y + 20, section.section);
			painter.drawText(QRect(amountX, y, 150, 26), Qt::AlignRight | Qt::AlignVCenter, money(section.total));
			y += 30;
			painter.setFont(normalFont);

			if (section.tags.isEmpty()) {
				painter.setPen(QColor(95, 95, 95));
				painter.drawText(margin + 28, y + 16, "No tag");
				painter.drawText(QRect(amountX, y, 150, 20), Qt::AlignRight | Qt::AlignVCenter, money(section.total));
				painter.setPen(Qt::black);
				y += 24;
			}
			else {
				for (const SummaryItem& tag : section.tags) {
					painter.setPen(QColor(95, 95, 95));
					painter.drawText(margin + 28, y + 16, tag.tag);
					painter.drawText(QRect(amountX, y, 150, 20), Qt::AlignRight | Qt::AlignVCenter, money(tag.total));
					painter.setPen(Qt::black);
					y += 24;
				}
			}

			painter.setPen(QColor(210, 210, 210));
			painter.drawLine(margin, y + 4, margin + pageWidth, y + 4);
			painter.setPen(Qt::black);
			y += 22;
		}
		y += 18;
	};

	drawSectionGroup("Income", incomeSections, QColor(220, 238, 226));
	drawSectionGroup("Expenses", expenseSections, QColor(246, 224, 224));

	painter.end();
	return true;
}

bool ReportExporter::exportXlsx(const QString& filePath, const ReportSession& session, const BudgetFormat& format, QString* error) const
{
	const QVector<SummaryItem> items = summaryItems(session, format);
	const QVector<SectionSummary> incomeSections = sectionSummaries(items, "Income");
	const QVector<SectionSummary> expenseSections = sectionSummaries(items, "Expenses");
	QVector<QStringList> summaryRows;
	QVector<QStringList> incomeRows;
	QVector<QStringList> expenseRows;
	double incomeTotal = 0.0;
	double expenseTotal = 0.0;

	for (const SummaryItem& item : items) {
		if (item.root == "Income") {
			incomeTotal += item.total;
		}
		else if (item.root == "Expenses") {
			expenseTotal += item.total;
		}
	}

	for (const SectionSummary& section : incomeSections) {
		if (section.total == 0.0) {
			continue;
		}

		incomeRows.append({ section.section, "", QString::number(section.total, 'f', 2) });

		for (const SummaryItem& tag : section.tags) {
			incomeRows.append({ "", tag.tag, QString::number(tag.total, 'f', 2) });
		}
	}

	for (const SectionSummary& section : expenseSections) {
		if (section.total == 0.0) {
			continue;
		}

		expenseRows.append({ section.section, "", QString::number(section.total, 'f', 2) });

		for (const SummaryItem& tag : section.tags) {
			expenseRows.append({ "", tag.tag, QString::number(tag.total, 'f', 2) });
		}
	}

	const int rowCount = qMax(incomeRows.size(), expenseRows.size());

	for (int i = 0; i < rowCount; ++i) {
		const QString incomeName = i < incomeRows.size() ? incomeRows[i][0] : "";
		const QString incomeTag = i < incomeRows.size() ? incomeRows[i][1] : "";
		const QString incomeAmount = i < incomeRows.size() ? incomeRows[i][2] : "";
		const QString expenseName = i < expenseRows.size() ? expenseRows[i][0] : "";
		const QString expenseTag = i < expenseRows.size() ? expenseRows[i][1] : "";
		const QString expenseAmount = i < expenseRows.size() ? expenseRows[i][2] : "";
		summaryRows.append({ incomeName, incomeTag, incomeAmount, "", expenseName, expenseTag, expenseAmount });
	}

	summaryRows.append({ "", "", "", "", "", "", "" });
	summaryRows.append({ "Total Money In", "", QString::number(incomeTotal, 'f', 2), "", "Total Expenses", "", QString::number(expenseTotal, 'f', 2) });
	summaryRows.append({ "Net", "", QString::number(incomeTotal - expenseTotal, 'f', 2), "", "", "", "" });

	QVector<QStringList> detailRows;

	for (const Transaction& transaction : session.transactions()) {
		const QStringList pathParts = transaction.assignedPath.split(" / ", Qt::SkipEmptyParts);
		const QString note = pathParts.size() > 2 ? pathParts.mid(2).join(" / ") : "";

		detailRows.append({
			transaction.sourceFile,
			transaction.date.toString("yyyy-MM-dd"),
			note,
			QString::number(transaction.amount, 'f', 2),
			transaction.skipped ? "Skipped" : transaction.assignedPath
		});
	}

	QVector<ZipEntry> entries;
	entries.append({ "[Content_Types].xml", QByteArray(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>
<Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/worksheets/sheet2.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
</Types>)") });
	entries.append({ "_rels/.rels", QByteArray(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/>
</Relationships>)") });
	entries.append({ "docProps/app.xml", QByteArray(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties" xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">
<Application>ExpenseBot</Application></Properties>)") });
	entries.append({ "docProps/core.xml", ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
		"<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:dcterms=\"http://purl.org/dc/terms/\" xmlns:dcmitype=\"http://purl.org/dc/dcmitype/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
		"<dc:creator>ExpenseBot</dc:creator><cp:lastModifiedBy>ExpenseBot</cp:lastModifiedBy><dcterms:created xsi:type=\"dcterms:W3CDTF\">" + QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toUtf8() + "</dcterms:created></cp:coreProperties>") });
	entries.append({ "xl/workbook.xml", QByteArray(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="Summary" sheetId="1" r:id="rId1"/><sheet name="Transactions" sheetId="2" r:id="rId2"/></sheets>
</workbook>)") });
	entries.append({ "xl/_rels/workbook.xml.rels", QByteArray(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet2.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>)") });
	entries.append({ "xl/styles.xml", QByteArray(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>
<fills count="1"><fill><patternFill patternType="none"/></fill></fills>
<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/></cellXfs>
</styleSheet>)") });
	entries.append({ "xl/worksheets/sheet1.xml", worksheetXml({ "Income Section", "Tag / Note", "Amount", "", "Expense Section", "Tag / Note", "Amount" }, summaryRows, { 3, 7 }) });
	entries.append({ "xl/worksheets/sheet2.xml", worksheetXml({ "Report", "Date", "Note", "Original Amount", "Assigned Section" }, detailRows, { 4 }) });

	return writeZip(filePath, entries, error);
}

QVector<ReportExporter::SummaryItem> ReportExporter::summaryItems(const ReportSession& session, const BudgetFormat& format)
{
	QVector<SummaryItem> items;

	for (const ReportSummaryRow& row : session.summaryRows(format)) {
		if (row.total == 0.0) {
			continue;
		}

		const QStringList parts = row.path.split(" / ", Qt::SkipEmptyParts);

		if (parts.size() < 2) {
			continue;
		}

		SummaryItem item;
		item.root = parts[0];
		item.section = parts[1];

		if (parts.size() > 2) {
			item.tag = parts.mid(2).join(" / ");
		}

		item.total = row.total;
		items.append(item);
	}

	return items;
}

QVector<ReportExporter::SectionSummary> ReportExporter::sectionSummaries(const QVector<SummaryItem>& items, const QString& root)
{
	QVector<SectionSummary> sections;
	QHash<QString, int> indexBySection;

	for (const SummaryItem& item : items) {
		if (item.root != root) {
			continue;
		}

		if (!indexBySection.contains(item.section)) {
			SectionSummary section;
			section.root = item.root;
			section.section = item.section;
			indexBySection.insert(item.section, sections.size());
			sections.append(section);
		}

		SectionSummary& section = sections[indexBySection.value(item.section)];
		section.total += item.total;

		if (!item.tag.isEmpty()) {
			section.tags.append(item);
		}
	}

	return sections;
}

QString ReportExporter::money(double value)
{
	const QString prefix = value < 0 ? "-$" : "$";
	return prefix + QString::number(std::abs(value), 'f', 2);
}

QString ReportExporter::xmlEscape(const QString& value)
{
	QString escaped = value;
	escaped.replace('&', "&amp;");
	escaped.replace('<', "&lt;");
	escaped.replace('>', "&gt;");
	escaped.replace('"', "&quot;");
	escaped.replace('\'', "&apos;");
	return escaped;
}

QString ReportExporter::cellText(const QString& cell, const QString& value)
{
	return "<c r=\"" + cell + "\" t=\"inlineStr\"><is><t>" + xmlEscape(value) + "</t></is></c>";
}

QString ReportExporter::cellNumber(const QString& cell, double value)
{
	return "<c r=\"" + cell + "\"><v>" + QString::number(value, 'f', 2) + "</v></c>";
}

QString ReportExporter::columnName(int column)
{
	QString name;

	while (column > 0) {
		--column;
		name.prepend(QChar('A' + (column % 26)));
		column /= 26;
	}

	return name;
}

QByteArray ReportExporter::worksheetXml(const QStringList& headers, const QVector<QStringList>& rows, const QVector<int>& numberColumns)
{
	QString xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
		"<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheetData>";

	xml += "<row r=\"1\">";

	for (int column = 1; column <= headers.size(); ++column) {
		xml += cellText(columnName(column) + "1", headers[column - 1]);
	}

	xml += "</row>";

	for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex) {
		const int excelRow = rowIndex + 2;
		xml += "<row r=\"" + QString::number(excelRow) + "\">";
		const QStringList row = rows[rowIndex];

		for (int column = 1; column <= row.size(); ++column) {
			const QString cell = columnName(column) + QString::number(excelRow);

			if (numberColumns.contains(column)) {
				bool ok = false;
				const double value = row[column - 1].toDouble(&ok);
				xml += ok ? cellNumber(cell, value) : cellText(cell, row[column - 1]);
			}
			else {
				xml += cellText(cell, row[column - 1]);
			}
		}

		xml += "</row>";
	}

	xml += "</sheetData></worksheet>";
	return xml.toUtf8();
}

quint32 ReportExporter::crc32(const QByteArray& data)
{
	quint32 crc = 0xFFFFFFFF;

	for (uchar byte : data) {
		crc ^= byte;

		for (int bit = 0; bit < 8; ++bit) {
			crc = (crc >> 1) ^ (0xEDB88320 & (0 - (crc & 1)));
		}
	}

	return ~crc;
}

bool ReportExporter::writeZip(const QString& filePath, QVector<ZipEntry> entries, QString* error)
{
	QFile file(filePath);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		if (error != nullptr) {
			*error = "Could not write XLSX file.";
		}
		return false;
	}

	auto write16 = [&](quint16 value) {
		file.putChar(static_cast<char>(value & 0xFF));
		file.putChar(static_cast<char>((value >> 8) & 0xFF));
	};
	auto write32 = [&](quint32 value) {
		write16(static_cast<quint16>(value & 0xFFFF));
		write16(static_cast<quint16>((value >> 16) & 0xFFFF));
	};

	for (ZipEntry& entry : entries) {
		entry.offset = static_cast<quint32>(file.pos());
		entry.crc = crc32(entry.data);
		const QByteArray name = entry.name.toUtf8();

		write32(0x04034b50);
		write16(20);
		write16(0);
		write16(0);
		write16(0);
		write16(0);
		write32(entry.crc);
		write32(entry.data.size());
		write32(entry.data.size());
		write16(name.size());
		write16(0);
		file.write(name);
		file.write(entry.data);
	}

	const quint32 centralOffset = static_cast<quint32>(file.pos());

	for (const ZipEntry& entry : entries) {
		const QByteArray name = entry.name.toUtf8();

		write32(0x02014b50);
		write16(20);
		write16(20);
		write16(0);
		write16(0);
		write16(0);
		write16(0);
		write32(entry.crc);
		write32(entry.data.size());
		write32(entry.data.size());
		write16(name.size());
		write16(0);
		write16(0);
		write16(0);
		write16(0);
		write32(0);
		write32(entry.offset);
		file.write(name);
	}

	const quint32 centralSize = static_cast<quint32>(file.pos()) - centralOffset;

	write32(0x06054b50);
	write16(0);
	write16(0);
	write16(entries.size());
	write16(entries.size());
	write32(centralSize);
	write32(centralOffset);
	write16(0);

	return true;
}
