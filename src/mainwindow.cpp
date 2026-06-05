#include "mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>

#include <QInputDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QLineEdit>

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

	auto* buttonLayout = new QHBoxLayout();

	addSectionButton = new QPushButton("Add Section", formatPage);
	addBabySectionButton = new QPushButton("Add Baby Section", formatPage);
	renameSectionButton = new QPushButton("Rename", formatPage);
	removeSectionButton = new QPushButton("Remove", formatPage);
	backButton = new QPushButton("Back", formatPage);

	buttonLayout->addWidget(addSectionButton);
	buttonLayout->addWidget(addBabySectionButton);
	buttonLayout->addWidget(renameSectionButton);
	buttonLayout->addWidget(removeSectionButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(backButton);

	layout->addWidget(titleLabel);
	layout->addWidget(formatTree);
	layout->addLayout(buttonLayout);

	pageStack->addWidget(formatPage);

	connect(addSectionButton, &QPushButton::clicked, this, &MainWindow::addSection);
	connect(addBabySectionButton, &QPushButton::clicked, this, &MainWindow::addBabySection);
	connect(renameSectionButton, &QPushButton::clicked, this, &MainWindow::renameSection);
	connect(removeSectionButton, &QPushButton::clicked, this, &MainWindow::removeSection);
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

	for (int i = 0; i < budgetFormat.roots().size(); ++i) {
		addSectionToTree(nullptr, budgetFormat.roots()[i], QVector<int>{ i });
	}

	formatTree->expandAll();
}

void MainWindow::addSectionToTree(QTreeWidgetItem* parentItem, const BudgetSection& section, const QVector<int>& path) {
	auto* item = new QTreeWidgetItem();
	item->setText(0, section.locked ? section.name + " (locked)" : section.name);

	item->setData(0, Qt::UserRole, QVariant::fromValue(path));

	if (parentItem == nullptr) {
		formatTree->addTopLevelItem(item);
	}
	else {
		parentItem->addChild(item);
	}

	for (int i = 0; i < section.children.size(); ++i) {
		QVector<int> childPath = path;
		childPath.append(i);
		addSectionToTree(item, section.children[i], childPath);
	}
}

int MainWindow::depthForItem(QTreeWidgetItem* item) const {
	int depth = 0;

	while (item != nullptr && item->parent() != nullptr) {
		++depth;
		item = item->parent();
	}

	return depth;
}

BudgetSection* MainWindow::sectionForItem(QTreeWidgetItem* item) {
	if (item == nullptr) {
		return nullptr;
	}

	QVector<int> path = item->data(0, Qt::UserRole).value<QVector<int>>();

	QVector<BudgetSection>* currentList = &budgetFormat.roots();
	BudgetSection* currentSection = nullptr;

	for (int index : path) {
		if (index < 0 || index >= currentList->size()) {
			return nullptr;
		}

		currentSection = &(*currentList)[index];
		currentList = &currentSection->children;
	}

	return currentSection;
}

QVector<BudgetSection>* MainWindow::siblingListForItem(QTreeWidgetItem* item) {
	if (item == nullptr) {
		return nullptr;
	}

	QVector<int> path = item->data(0, Qt::UserRole).value<QVector<int>>();

	if (path.isEmpty()) {
		return nullptr;
	}

	path.removeLast();

	QVector<BudgetSection>* currentList = &budgetFormat.roots();

	for (int index : path) {
		if (index < 0 || index >= currentList->size()) {
			return nullptr;
		}

		currentList = &(*currentList)[index].children;
	}

	return currentList;
}

void MainWindow::addSection() {
	QTreeWidgetItem* selectedItem = formatTree->currentItem();

	if (selectedItem == nullptr) {
		QMessageBox::warning(this, "No Selection", "Select Income or Expenses first.");
		return;
	}

	if (depthForItem(selectedItem) != 0) {
		QMessageBox::warning(this, "Invalid Selection", "Sections can only be added under Income or Expenses.");
		return;
	}

	bool ok = false;
	QString name = QInputDialog::getText(this, "Add Section", "Section name:", QLineEdit::Normal, "", &ok);

	if (!ok || name.trimmed().isEmpty()) {
		return;
	}

	BudgetSection* parentSection = sectionForItem(selectedItem);
	parentSection->children.append(BudgetSection{ name.trimmed(), false, {} });

	populateFormatTree();
}

void MainWindow::addBabySection() {
	QTreeWidgetItem* selectedItem = formatTree->currentItem();

	if (selectedItem == nullptr) {
		QMessageBox::warning(this, "No Selection", "Select a child section first.");
		return;
	}

	if (depthForItem(selectedItem) != 1) {
		QMessageBox::warning(this, "Invalid Selection", "Baby sections can only be added under child sections.");
		return;
	}

	bool ok = false;
	QString name = QInputDialog::getText(this, "Add Baby Section", "Baby section name:", QLineEdit::Normal, "", &ok);

	if (!ok || name.trimmed().isEmpty()) {
		return;
	}

	BudgetSection* parentSection = sectionForItem(selectedItem);
	parentSection->children.append(BudgetSection{ name.trimmed(), false, {} });

	populateFormatTree();
}

void MainWindow::renameSection() {
	QTreeWidgetItem* selectedItem = formatTree->currentItem();
	BudgetSection* section = sectionForItem(selectedItem);

	if (section == nullptr) {
		return;
	}

	if (section->locked) {
		QMessageBox::warning(this, "Locked Section", "Income and Expenses cannot be renamed.");
		return;
	}

	bool ok = false;
	QString name = QInputDialog::getText(this, "Rename Section", "New name:", QLineEdit::Normal, section->name, &ok);

	if (!ok || name.trimmed().isEmpty()) {
		return;
	}

	section->name = name.trimmed();

	populateFormatTree();
}

void MainWindow::removeSection() {
	QTreeWidgetItem* selectedItem = formatTree->currentItem();
	BudgetSection* section = sectionForItem(selectedItem);

	if (section == nullptr) {
		return;
	}

	if (section->locked) {
		QMessageBox::warning(this, "Locked Section", "Income and Expenses cannot be removed.");
		return;
	}

	QVector<int> path = selectedItem->data(0, Qt::UserRole).value<QVector<int>>();
	QVector<BudgetSection>* siblings = siblingListForItem(selectedItem);

	if (siblings == nullptr || path.isEmpty()) {
		return;
	}

	siblings->removeAt(path.last());

	populateFormatTree();
}