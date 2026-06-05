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
	void addSectionToTree(QTreeWidgetItem* parentItem, const BudgetSection& section, const QVector<int>& path);
	BudgetSection* sectionForItem(QTreeWidgetItem* item);
	QVector<BudgetSection>* siblingListForItem(QTreeWidgetItem* item);
	int depthForItem(QTreeWidgetItem* item) const;

	void addSection();
	void addBabySection();
	void renameSection();
	void removeSection();

	BudgetFormat budgetFormat;

	QStackedWidget* pageStack = nullptr;

	QPushButton* createReportButton = nullptr;
	QPushButton* editFormatButton = nullptr;
	QPushButton* backButton = nullptr;

	QPushButton* addSectionButton = nullptr;
	QPushButton* addBabySectionButton = nullptr;
	QPushButton* renameSectionButton = nullptr;
	QPushButton* removeSectionButton = nullptr;

	QTreeWidget* formatTree = nullptr;
};