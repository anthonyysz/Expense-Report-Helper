#pragma once

#include <QString>
#include <QVector>

struct BudgetSection {
	QString name;
	bool locked = false;
	QVector<BudgetSection> children;
};

class BudgetFormat {
public:
	BudgetFormat();

	QVector<BudgetSection>& roots();
	const QVector<BudgetSection>& roots() const;

private:
	QVector<BudgetSection> rootSections;
};