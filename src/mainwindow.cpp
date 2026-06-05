#include "mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
	setWindowTitle("ExpenseBot");
	resize(1000, 700);

	pageStack = new QStackedWidget(this);

	buildLandingPage();
	buildFormatPage();

	setCentralWidget(pageStack);
	showLandingPage();
}

void MainWindow::buildLandingPage() {
	auto* centralWidget = new QWidget(pageStack);
	auto* layout = new QVBoxLayout(centralWidget);

	auto* titleLabel = new QLabel("ExpenseBot", centralWidget);
	createReportButton = new QPushButton("Create Report", centralWidget);
	editFormatButton = new QPushButton("Edit Format", centralWidget);

	layout->addWidget(titleLabel);
	layout->addWidget(createReportButton);
	layout->addWidget(editFormatButton);
	layout->addStretch();

	pageStack->addWidget(centralWidget);

	connect(editFormatButton, &QPushButton::clicked, this, &MainWindow::showFormatPage);
}

void MainWindow::buildFormatPage() {
	auto* formatPage = new QWidget(pageStack);
	auto* layout = new QVBoxLayout(formatPage);

	auto titleLabel = new QLabel("Edit Format", formatPage);

	formatTree = new QTreeWidget(formatPage);
	formatTree->setHeaderLabel("Budget Sections");
	formatTree->header()->setStretchLastSection(true);

	backButton = new QPushButton("Back", formatPage);

	layout->addWidget(titleLabel);
	layout->addWidget(formatTree);
	layout->addWidget(backButton);

	pageStack->addWidget(formatPage);

	connect(backButton, &QPushButton::clicked, this, &MainWindow::showLandingPage);
}

void MainWindow::showLandingPage() {
	pageStack->setCurrentIndex(0);
}

void MainWindow::showFormatPage() {
	populateFormatTree();
	pageStack->setCurrentIndex(1);
}

void MainWindow::populateFormatTree() {
	formatTree->clear();

	for (const BudgetSection& section : budgetFormat.roots()) {
		addSectionToTree(nullptr, section);
	}

	formatTree->expandAll();
}

void MainWindow::addSectionToTree(QTreeWidgetItem* parentItem, const BudgetSection& section) {
	auto* item = new QTreeWidgetItem();
	item->setText(0, section.name);

	if (section.locked) {
		item->setText(0, section.name + " (locked)");
	}

	if (parentItem == nullptr) {
		formatTree->addTopLevelItem(item);
	}
	else {
		parentItem->addChild(item);
	}

	for (const BudgetSection& child : section.children) {
		addSectionToTree(item, child);
	}
}