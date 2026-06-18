#pragma once

#include <QDate>
#include <QString>
#include <QStringList>
#include <QVector>

struct Transaction
{
	QString sourceFile;
	QDate date;
	QString description;
	double amount = 0.0;
	QString assignedPath;
	bool skipped = false;
};

struct ReportSummaryRow
{
	QString path;
	double total = 0.0;
	QVector<Transaction> transactions;
};
