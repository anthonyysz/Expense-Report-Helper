#include "budgetformat.h"

BudgetFormat::BudgetFormat() {
	BudgetSection income;
	income.name = "Income";
	income.locked = true;
	income.children = {
		BudgetSection{ "Earned", false, {} },
		BudgetSection{ "Received", false, {} }
	};

	BudgetSection expenses;
	expenses.name = "Expenses";
	expenses.locked = true;
	expenses.children = {
		BudgetSection{ "Necessities", false, {} },
		BudgetSection{ "Food", false, {} },
		BudgetSection{ "Other", false, {} }
	};

	rootSections = {
		income,
		expenses
	};
}

const QVector<BudgetSection>& BudgetFormat::roots() const {
	return rootSections;
}