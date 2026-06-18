#pragma once

#include "budgetformat.h"
#include "reportsession.h"

#include <QString>

class ReportExporter
{
public:
	bool exportPdf(const QString& filePath, const ReportSession& session, const BudgetFormat& format, QString* error = nullptr) const;
	bool exportXlsx(const QString& filePath, const ReportSession& session, const BudgetFormat& format, QString* error = nullptr) const;

private:
	struct SummaryItem
	{
		QString root;
		QString section;
		QString tag;
		double total = 0.0;
	};

	struct SectionSummary
	{
		QString root;
		QString section;
		double total = 0.0;
		QVector<SummaryItem> tags;
	};

	struct ZipEntry
	{
		QString name;
		QByteArray data;
		quint32 crc = 0;
		quint32 offset = 0;
	};

	static QVector<SummaryItem> summaryItems(const ReportSession& session, const BudgetFormat& format);
	static QVector<SectionSummary> sectionSummaries(const QVector<SummaryItem>& items, const QString& root);
	static QString money(double value);
	static QString xmlEscape(const QString& value);
	static QString cellText(const QString& cell, const QString& value);
	static QString cellNumber(const QString& cell, double value);
	static QString columnName(int column);
	static QByteArray worksheetXml(const QStringList& headers, const QVector<QStringList>& rows, const QVector<int>& numberColumns);
	static quint32 crc32(const QByteArray& data);
	static bool writeZip(const QString& filePath, QVector<ZipEntry> entries, QString* error);
};
