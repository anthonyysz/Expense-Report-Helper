#pragma once

#include "budgetformat.h"
#include "transaction.h"

#include <QHash>
#include <QStringList>
#include <QVector>

class ReportSession
{
public:
	void setTransactions(const QVector<Transaction>& transactions);

	bool hasTransactions() const;
	bool isComplete() const;
	int currentIndex() const;
	int totalCount() const;
	const Transaction* currentTransaction() const;
	const QVector<Transaction>& transactions() const;

	void assignCurrent(const QString& sectionPath);
	void skipCurrent();
	void addManualTransaction(const Transaction& transaction);
	bool undo();
	QVector<ReportSummaryRow> summaryRows(const BudgetFormat& format) const;

private:
	QVector<QString> leafPaths(const BudgetSection& section, const QString& prefix) const;

	QVector<Transaction> reportTransactions;
	int transactionIndex = 0;
	QVector<int> assignmentHistory;
};
