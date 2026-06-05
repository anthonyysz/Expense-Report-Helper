#pragma once

#include <QMainWindow>

#include "budgetformat.h"

class QPushButton;
class QStackedWidget;
class QTreeWidget;
class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
public:
	explicit MainWindow(QWidget* parent = nullptr);

private:
	void buildLandingPage();
	void buildFormatPage();
	void showLandingPage();
	void showFormatPage();
	void populateFormatTree();
	void addSectionToTree(QTreeWidgetItem* parentItem, const BudgetSection& section);

	BudgetFormat budgetFormat;

	QStackedWidget* pageStack = nullptr;

	QPushButton* createReportButton = nullptr;
	QPushButton* editFormatButton = nullptr;
	QPushButton* backButton = nullptr;

	QTreeWidget* formatTree = nullptr;
};