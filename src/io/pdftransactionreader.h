#pragma once

#include "transaction.h"

#include <QString>
#include <QVector>

class PdfTransactionReader
{
public:
	QVector<Transaction> readTransactions(const QStringList& filePaths, QStringList* errors = nullptr) const;

private:
	QVector<Transaction> readFile(const QString& filePath, QStringList* errors) const;
	QVector<Transaction> parseText(const QString& filePath, const QString& text) const;
	QVector<Transaction> parseBankOfAmericaText(const QString& filePath, const QString& text) const;
	bool appendBankOfAmericaRecord(QVector<Transaction>& transactions, const QString& sourceName, const QString& record) const;
	QVector<Transaction> parseCapitalOneText(const QString& filePath, const QString& text) const;
	bool appendCapitalOneRecord(QVector<Transaction>& transactions, const QString& sourceName, const QString& record, int year) const;
};
