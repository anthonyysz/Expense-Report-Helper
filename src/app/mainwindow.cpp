#include "mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QFont>
#include <QHash>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	setWindowTitle("ExpenseBot");
	resize(1100, 760);

	budgetFormat.loadFromFile(formatFilePath());

	pageStack = new QStackedWidget(this);

	buildLandingPage();
	buildFormatPage();
	buildReportPage();
	buildSortPage();
	buildReviewPage();

	setCentralWidget(pageStack);
	showLandingPage();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
	if (pageStack->currentIndex() == 3) {
		if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Left) {
			undoAssignment();
			return;
		}

		if (event->key() == Qt::Key_Right) {
			skipTransaction();
			return;
		}

		if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
			assignSelectedTransaction();
			return;
		}
	}

	QMainWindow::keyPressEvent(event);
}

void MainWindow::buildLandingPage()
{
	auto* page = new QWidget(pageStack);
	auto* layout = new QVBoxLayout(page);

	auto* titleLabel = new QLabel("ExpenseBot", page);
	createReportButton = new QPushButton("Create Report", page);
	editFormatButton = new QPushButton("Edit Format", page);

	titleLabel->setAlignment(Qt::AlignCenter);
	createReportButton->setMinimumHeight(44);
	editFormatButton->setMinimumHeight(44);

	layout->addStretch();
	layout->addWidget(titleLabel);
	layout->addWidget(createReportButton);
	layout->addWidget(editFormatButton);
	layout->addStretch();

	pageStack->addWidget(page);

	connect(createReportButton, &QPushButton::clicked, this, &MainWindow::showReportPage);
	connect(editFormatButton, &QPushButton::clicked, this, &MainWindow::showFormatPage);
}

void MainWindow::buildFormatPage()
{
	auto* page = new QWidget(pageStack);
	auto* layout = new QVBoxLayout(page);
	auto* buttonLayout = new QHBoxLayout();

	auto* titleLabel = new QLabel("Edit Format", page);
	formatTree = new QTreeWidget(page);
	addSectionButton = new QPushButton("Add Section", page);
	addBabySectionButton = new QPushButton("Add Baby Section", page);
	renameSectionButton = new QPushButton("Rename", page);
	removeSectionButton = new QPushButton("Remove", page);
	formatBackButton = new QPushButton("Back", page);

	formatTree->setHeaderLabel("Budget Sections");
	formatTree->header()->setStretchLastSection(true);

	buttonLayout->addWidget(addSectionButton);
	buttonLayout->addWidget(addBabySectionButton);
	buttonLayout->addWidget(renameSectionButton);
	buttonLayout->addWidget(removeSectionButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(formatBackButton);

	layout->addWidget(titleLabel);
	layout->addWidget(formatTree);
	layout->addLayout(buttonLayout);

	pageStack->addWidget(page);

	connect(addSectionButton, &QPushButton::clicked, this, &MainWindow::addSection);
	connect(addBabySectionButton, &QPushButton::clicked, this, &MainWindow::addBabySection);
	connect(renameSectionButton, &QPushButton::clicked, this, &MainWindow::renameSection);
	connect(removeSectionButton, &QPushButton::clicked, this, &MainWindow::removeSection);
	connect(formatBackButton, &QPushButton::clicked, this, &MainWindow::showLandingPage);
}

void MainWindow::buildReportPage()
{
	auto* page = new QWidget(pageStack);
	auto* layout = new QVBoxLayout(page);
	auto* buttonLayout = new QHBoxLayout();

	auto* titleLabel = new QLabel("Create Report", page);
	pdfFileList = new QListWidget(page);
	addPdfFilesButton = new QPushButton("Add PDF Statements", page);
	clearPdfFilesButton = new QPushButton("Clear", page);
	startReportButton = new QPushButton("Start", page);
	reportBackButton = new QPushButton("Back", page);

	pdfFileList->setSelectionMode(QAbstractItemView::NoSelection);

	buttonLayout->addWidget(addPdfFilesButton);
	buttonLayout->addWidget(clearPdfFilesButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(reportBackButton);
	buttonLayout->addWidget(startReportButton);

	layout->addWidget(titleLabel);
	layout->addWidget(pdfFileList);
	layout->addLayout(buttonLayout);

	pageStack->addWidget(page);

	connect(addPdfFilesButton, &QPushButton::clicked, this, &MainWindow::choosePdfFiles);
	connect(clearPdfFilesButton, &QPushButton::clicked, this, &MainWindow::clearPdfFiles);
	connect(startReportButton, &QPushButton::clicked, this, &MainWindow::startReport);
	connect(reportBackButton, &QPushButton::clicked, this, &MainWindow::showLandingPage);
}

void MainWindow::buildSortPage()
{
	auto* page = new QWidget(pageStack);
	auto* outerLayout = new QVBoxLayout(page);
	auto* bodyLayout = new QHBoxLayout();
	auto* transactionLayout = new QVBoxLayout();
	auto* sortingLayout = new QVBoxLayout();
	auto* sortingButtonLayout = new QHBoxLayout();
	auto* buttonLayout = new QHBoxLayout();

	progressLabel = new QLabel(page);
	sourceFileLabel = new QLabel(page);
	dateLabel = new QLabel(page);
	descriptionLabel = new QLabel(page);
	amountLabel = new QLabel(page);
	sortingTree = new QTreeWidget(page);
	nextButton = new QPushButton("Next", page);
	skipButton = new QPushButton("Skip", page);
	undoButton = new QPushButton("Undo", page);
	sortAddSectionButton = new QPushButton("Add Section", page);
	sortAddBabySectionButton = new QPushButton("Add Baby Section", page);
	sortRenameSectionButton = new QPushButton("Rename", page);
	sortRemoveSectionButton = new QPushButton("Remove", page);
	reviewButton = new QPushButton("Review Report", page);
	sortBackButton = new QPushButton("Back", page);

	descriptionLabel->setWordWrap(true);
	sortingTree->setHeaderLabel("Place Transaction");
	sortingTree->header()->setStretchLastSection(true);

	transactionLayout->addWidget(progressLabel);
	transactionLayout->addWidget(sourceFileLabel);
	transactionLayout->addWidget(dateLabel);
	transactionLayout->addWidget(descriptionLabel);
	transactionLayout->addWidget(amountLabel);
	transactionLayout->addStretch();

	sortingButtonLayout->addWidget(sortAddSectionButton);
	sortingButtonLayout->addWidget(sortAddBabySectionButton);
	sortingButtonLayout->addWidget(sortRenameSectionButton);
	sortingButtonLayout->addWidget(sortRemoveSectionButton);

	sortingLayout->addWidget(sortingTree);
	sortingLayout->addLayout(sortingButtonLayout);

	bodyLayout->addLayout(transactionLayout, 2);
	bodyLayout->addLayout(sortingLayout, 1);

	buttonLayout->addWidget(undoButton);
	buttonLayout->addWidget(skipButton);
	buttonLayout->addWidget(nextButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(reviewButton);
	buttonLayout->addWidget(sortBackButton);

	outerLayout->addLayout(bodyLayout);
	outerLayout->addLayout(buttonLayout);

	pageStack->addWidget(page);

	connect(nextButton, &QPushButton::clicked, this, &MainWindow::assignSelectedTransaction);
	connect(skipButton, &QPushButton::clicked, this, &MainWindow::skipTransaction);
	connect(undoButton, &QPushButton::clicked, this, &MainWindow::undoAssignment);
	connect(sortAddSectionButton, &QPushButton::clicked, this, &MainWindow::addSectionDuringSorting);
	connect(sortAddBabySectionButton, &QPushButton::clicked, this, &MainWindow::addBabySectionDuringSorting);
	connect(sortRenameSectionButton, &QPushButton::clicked, this, &MainWindow::renameSectionDuringSorting);
	connect(sortRemoveSectionButton, &QPushButton::clicked, this, &MainWindow::removeSectionDuringSorting);
	connect(reviewButton, &QPushButton::clicked, this, &MainWindow::showReviewPage);
	connect(sortBackButton, &QPushButton::clicked, this, &MainWindow::showReportPage);

	skipButton->setShortcut(QKeySequence(Qt::Key_Right));
	undoButton->setShortcut(QKeySequence(Qt::Key_Left));
}

void MainWindow::buildReviewPage()
{
	auto* page = new QWidget(pageStack);
	auto* layout = new QVBoxLayout(page);
	auto* buttonLayout = new QHBoxLayout();

	auto* titleLabel = new QLabel("Review Report", page);
	reviewTree = new QTreeWidget(page);
	addManualTransactionButton = new QPushButton("Add Transaction", page);
	exportPdfButton = new QPushButton("Export PDF", page);
	exportXlsxButton = new QPushButton("Export XLSX", page);
	reviewBackButton = new QPushButton("Back to Sorting", page);

	reviewTree->setColumnCount(4);
	reviewTree->setHeaderLabels({ "Section / Date", "Tag / Description", "Amount", "Source" });
	reviewTree->header()->setStretchLastSection(true);
	reviewTree->setAlternatingRowColors(true);

	buttonLayout->addWidget(addManualTransactionButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(reviewBackButton);
	buttonLayout->addWidget(exportPdfButton);
	buttonLayout->addWidget(exportXlsxButton);

	layout->addWidget(titleLabel);
	layout->addWidget(reviewTree);
	layout->addLayout(buttonLayout);

	pageStack->addWidget(page);

	connect(addManualTransactionButton, &QPushButton::clicked, this, &MainWindow::addManualTransaction);
	connect(reviewBackButton, &QPushButton::clicked, this, &MainWindow::showSortPage);
	connect(exportPdfButton, &QPushButton::clicked, this, &MainWindow::exportPdf);
	connect(exportXlsxButton, &QPushButton::clicked, this, &MainWindow::exportXlsx);
}

void MainWindow::showLandingPage()
{
	pageStack->setCurrentIndex(0);
}

void MainWindow::showFormatPage()
{
	populateFormatTree();
	pageStack->setCurrentIndex(1);
}

void MainWindow::showReportPage()
{
	pageStack->setCurrentIndex(2);
}

void MainWindow::showSortPage()
{
	populateSortingTree();
	updateSortPage();
	pageStack->setCurrentIndex(3);
	sortingTree->setFocus();
}

void MainWindow::showReviewPage()
{
	populateReviewTree();
	pageStack->setCurrentIndex(4);
}

QString MainWindow::formatFilePath() const
{
	const QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	QDir directory(appDataPath);

	if (!directory.exists()) {
		directory.mkpath(".");
	}

	return directory.filePath("budget_format.json");
}

void MainWindow::saveBudgetFormat()
{
	if (!budgetFormat.saveToFile(formatFilePath())) {
		QMessageBox::warning(this, "Save Failed", "ExpenseBot could not save your budget format.");
	}
}

void MainWindow::populateFormatTree()
{
	formatTree->clear();

	for (int i = 0; i < budgetFormat.roots().size(); ++i) {
		addSectionToTree(nullptr, budgetFormat.roots()[i], QVector<int>{ i });
	}

	formatTree->expandAll();

	if (formatTree->topLevelItemCount() > 0) {
		formatTree->setCurrentItem(formatTree->topLevelItem(0));
	}
}

void MainWindow::addSectionToTree(QTreeWidgetItem* parentItem, const BudgetSection& section, const QVector<int>& path)
{
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

BudgetSection* MainWindow::sectionForItem(QTreeWidgetItem* item)
{
	if (item == nullptr) {
		return nullptr;
	}

	QVector<int> path = item->data(0, Qt::UserRole).value<QVector<int>>();
	return sectionForPath(path);
}

BudgetSection* MainWindow::sectionForPath(const QVector<int>& path)
{
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

QVector<BudgetSection>* MainWindow::siblingListForItem(QTreeWidgetItem* item)
{
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

int MainWindow::depthForItem(QTreeWidgetItem* item) const
{
	int depth = 0;

	while (item != nullptr && item->parent() != nullptr) {
		++depth;
		item = item->parent();
	}

	return depth;
}

void MainWindow::addSection()
{
	QTreeWidgetItem* selectedItem = formatTree->currentItem();

	if (selectedItem == nullptr || depthForItem(selectedItem) != 0) {
		QMessageBox::warning(this, "Invalid Selection", "Select Income or Expenses first.");
		return;
	}

	bool ok = false;
	const QString name = QInputDialog::getText(this, "Add Section", "Section name:", QLineEdit::Normal, "", &ok).trimmed();

	if (!ok || name.isEmpty()) {
		return;
	}

	BudgetSection* parentSection = sectionForItem(selectedItem);
	parentSection->children.append(BudgetSection{ name, false, {} });

	saveBudgetFormat();
	populateFormatTree();
}

void MainWindow::addBabySection()
{
	QTreeWidgetItem* selectedItem = formatTree->currentItem();

	if (selectedItem == nullptr || depthForItem(selectedItem) != 1) {
		QMessageBox::warning(this, "Invalid Selection", "Baby sections can only be added under child sections.");
		return;
	}

	bool ok = false;
	const QString name = QInputDialog::getText(this, "Add Baby Section", "Baby section name:", QLineEdit::Normal, "", &ok).trimmed();

	if (!ok || name.isEmpty()) {
		return;
	}

	BudgetSection* parentSection = sectionForItem(selectedItem);
	parentSection->children.append(BudgetSection{ name, false, {} });

	saveBudgetFormat();
	populateFormatTree();
}

void MainWindow::renameSection()
{
	BudgetSection* section = sectionForItem(formatTree->currentItem());

	if (section == nullptr) {
		return;
	}

	if (section->locked) {
		QMessageBox::warning(this, "Locked Section", "Income and Expenses cannot be renamed.");
		return;
	}

	bool ok = false;
	const QString name = QInputDialog::getText(this, "Rename Section", "New name:", QLineEdit::Normal, section->name, &ok).trimmed();

	if (!ok || name.isEmpty()) {
		return;
	}

	section->name = name;
	saveBudgetFormat();
	populateFormatTree();
}

void MainWindow::removeSection()
{
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
	saveBudgetFormat();
	populateFormatTree();
}

void MainWindow::choosePdfFiles()
{
	const QStringList files = QFileDialog::getOpenFileNames(this, "Select PDF Statements", QString(), "PDF Files (*.pdf)");

	for (const QString& file : files) {
		if (!selectedPdfFiles.contains(file)) {
			selectedPdfFiles.append(file);
			pdfFileList->addItem(file);
		}
	}
}

void MainWindow::clearPdfFiles()
{
	selectedPdfFiles.clear();
	pdfFileList->clear();
}

void MainWindow::startReport()
{
	if (selectedPdfFiles.isEmpty()) {
		QMessageBox::warning(this, "No Statements", "Add at least one PDF statement first.");
		return;
	}

	QStringList errors;
	QVector<Transaction> transactions = pdfReader.readTransactions(selectedPdfFiles, &errors);

	if (transactions.isEmpty()) {
		QMessageBox::warning(this, "No Transactions", errors.isEmpty() ? "ExpenseBot could not find any transactions." : errors.join("\n"));
		return;
	}

	if (!errors.isEmpty()) {
		QMessageBox::warning(this, "Some Statements Need Review", errors.join("\n"));
	}

	reportSession.setTransactions(transactions);
	showSortPage();
}

void MainWindow::updateSortPage()
{
	const Transaction* transaction = reportSession.currentTransaction();
	const bool complete = reportSession.isComplete();

	progressLabel->setText(QString("Transaction %1 of %2").arg(qMin(reportSession.currentIndex() + 1, reportSession.totalCount())).arg(reportSession.totalCount()));
	nextButton->setEnabled(!complete);
	skipButton->setEnabled(!complete);
	reviewButton->setEnabled(complete);

	if (transaction == nullptr) {
		sourceFileLabel->setText("All transactions have been placed.");
		dateLabel->clear();
		descriptionLabel->clear();
		amountLabel->clear();
		return;
	}

	sourceFileLabel->setText("Report: " + transaction->sourceFile);
	dateLabel->setText("Date: " + transaction->date.toString("yyyy-MM-dd"));
	descriptionLabel->setText("Description: " + transaction->description);
	amountLabel->setText("Amount: $" + QString::number(std::abs(transaction->amount), 'f', 2));
}

void MainWindow::populateSortingTree()
{
	sortingTree->clear();

	for (int i = 0; i < budgetFormat.roots().size(); ++i) {
		addSortingSectionToTree(nullptr, budgetFormat.roots()[i], QString(), QVector<int>{ i });
	}

	sortingTree->expandAll();

	if (sortingTree->topLevelItemCount() > 0) {
		sortingTree->setCurrentItem(sortingTree->topLevelItem(0));
	}
}

void MainWindow::addSortingSectionToTree(QTreeWidgetItem* parentItem, const BudgetSection& section, const QString& path, const QVector<int>& indexPath)
{
	const QString currentPath = path.isEmpty() ? section.name : path + " / " + section.name;
	auto* item = new QTreeWidgetItem();
	item->setText(0, section.name);
	item->setData(0, Qt::UserRole, currentPath);
	item->setData(0, Qt::UserRole + 1, section.children.isEmpty());
	item->setData(0, Qt::UserRole + 2, QVariant::fromValue(indexPath));

	if (parentItem == nullptr) {
		sortingTree->addTopLevelItem(item);
	}
	else {
		parentItem->addChild(item);
	}

	for (int i = 0; i < section.children.size(); ++i) {
		QVector<int> childPath = indexPath;
		childPath.append(i);
		addSortingSectionToTree(item, section.children[i], currentPath, childPath);
	}
}

QString MainWindow::selectedSortingPath() const
{
	QTreeWidgetItem* item = sortingTree->currentItem();

	if (item == nullptr || !item->data(0, Qt::UserRole + 1).toBool()) {
		return {};
	}

	return item->data(0, Qt::UserRole).toString();
}

void MainWindow::assignSelectedTransaction()
{
	if (reportSession.isComplete()) {
		return;
	}

	const QString path = selectedSortingPath();

	if (path.isEmpty()) {
		QTreeWidgetItem* item = sortingTree->currentItem();

		if (item != nullptr) {
			item->setExpanded(!item->isExpanded());
		}
		return;
	}

	reportSession.assignCurrent(path);
	updateSortPage();
	sortingTree->setFocus();
}

void MainWindow::skipTransaction()
{
	if (reportSession.isComplete()) {
		return;
	}

	reportSession.skipCurrent();
	updateSortPage();
	sortingTree->setFocus();
}

void MainWindow::undoAssignment()
{
	if (!reportSession.undo()) {
		return;
	}

	updateSortPage();
	sortingTree->setFocus();
}

void MainWindow::addSectionDuringSorting()
{
	QTreeWidgetItem* selectedItem = sortingTree->currentItem();

	if (selectedItem == nullptr || depthForItem(selectedItem) != 0) {
		QMessageBox::warning(this, "Invalid Selection", "Select Income or Expenses first.");
		return;
	}

	bool ok = false;
	const QString name = QInputDialog::getText(this, "Add Section", "Section name:", QLineEdit::Normal, "", &ok).trimmed();

	if (!ok || name.isEmpty()) {
		return;
	}

	BudgetSection* parentSection = sectionForPath(selectedItem->data(0, Qt::UserRole + 2).value<QVector<int>>());

	if (parentSection == nullptr) {
		return;
	}

	parentSection->children.append(BudgetSection{ name, false, {} });
	saveBudgetFormat();
	populateSortingTree();
	sortingTree->setFocus();
}

void MainWindow::addBabySectionDuringSorting()
{
	QTreeWidgetItem* selectedItem = sortingTree->currentItem();

	if (selectedItem == nullptr || depthForItem(selectedItem) != 1) {
		QMessageBox::warning(this, "Invalid Selection", "Baby sections can only be added under child sections.");
		return;
	}

	bool ok = false;
	const QString name = QInputDialog::getText(this, "Add Baby Section", "Baby section name:", QLineEdit::Normal, "", &ok).trimmed();

	if (!ok || name.isEmpty()) {
		return;
	}

	BudgetSection* parentSection = sectionForPath(selectedItem->data(0, Qt::UserRole + 2).value<QVector<int>>());

	if (parentSection == nullptr) {
		return;
	}

	parentSection->children.append(BudgetSection{ name, false, {} });
	saveBudgetFormat();
	populateSortingTree();
	sortingTree->setFocus();
}

void MainWindow::renameSectionDuringSorting()
{
	QTreeWidgetItem* selectedItem = sortingTree->currentItem();
	BudgetSection* section = sectionForPath(selectedItem == nullptr ? QVector<int>() : selectedItem->data(0, Qt::UserRole + 2).value<QVector<int>>());

	if (section == nullptr) {
		return;
	}

	if (section->locked) {
		QMessageBox::warning(this, "Locked Section", "Income and Expenses cannot be renamed.");
		return;
	}

	bool ok = false;
	const QString name = QInputDialog::getText(this, "Rename Section", "New name:", QLineEdit::Normal, section->name, &ok).trimmed();

	if (!ok || name.isEmpty()) {
		return;
	}

	section->name = name;
	saveBudgetFormat();
	populateSortingTree();
	sortingTree->setFocus();
}

void MainWindow::removeSectionDuringSorting()
{
	QTreeWidgetItem* selectedItem = sortingTree->currentItem();

	if (selectedItem == nullptr) {
		return;
	}

	const QVector<int> path = selectedItem->data(0, Qt::UserRole + 2).value<QVector<int>>();
	BudgetSection* section = sectionForPath(path);

	if (section == nullptr) {
		return;
	}

	if (section->locked) {
		QMessageBox::warning(this, "Locked Section", "Income and Expenses cannot be removed.");
		return;
	}

	const QString sectionPath = selectedItem->data(0, Qt::UserRole).toString();

	for (const Transaction& transaction : reportSession.transactions()) {
		if (!transaction.assignedPath.isEmpty() && transaction.assignedPath.startsWith(sectionPath)) {
			QMessageBox::warning(this, "Section In Use", "This section already has sorted transactions in the current report.");
			return;
		}
	}

	if (QMessageBox::question(this, "Remove Section", "Remove this section from the format?") != QMessageBox::Yes) {
		return;
	}

	QVector<int> parentPath = path;
	parentPath.removeLast();
	QVector<BudgetSection>* siblings = &budgetFormat.roots();

	for (int index : parentPath) {
		if (index < 0 || index >= siblings->size()) {
			return;
		}

		siblings = &(*siblings)[index].children;
	}

	if (path.isEmpty() || path.last() < 0 || path.last() >= siblings->size()) {
		return;
	}

	siblings->removeAt(path.last());
	saveBudgetFormat();
	populateSortingTree();
	sortingTree->setFocus();
}

void MainWindow::populateReviewTree()
{
	reviewTree->clear();
	QHash<QString, QTreeWidgetItem*> sectionItems;
	QHash<QString, double> sectionTotals;

	for (const Transaction& transaction : reportSession.transactions()) {
		if (transaction.skipped || transaction.assignedPath.isEmpty()) {
			continue;
		}

		const QStringList parts = transaction.assignedPath.split(" / ", Qt::SkipEmptyParts);
		const QString root = parts.value(0);
		const QString section = parts.value(1);
		const QString tag = parts.size() > 2 ? parts.mid(2).join(" / ") : "";
		const QString sectionKey = root + " / " + section;

		if (!sectionItems.contains(sectionKey)) {
			auto* sectionItem = new QTreeWidgetItem(reviewTree);
			sectionItem->setText(0, sectionKey);
			QFont sectionFont = sectionItem->font(0);
			sectionFont.setBold(true);
			for (int column = 0; column < 4; ++column) {
				sectionItem->setFont(column, sectionFont);
			}
			sectionItems.insert(sectionKey, sectionItem);
			sectionTotals.insert(sectionKey, 0.0);
		}

		sectionTotals[sectionKey] += std::abs(transaction.amount);

		auto* transactionItem = new QTreeWidgetItem(sectionItems.value(sectionKey));
		transactionItem->setText(0, transaction.date.toString("yyyy-MM-dd"));
		transactionItem->setText(1, tag.isEmpty() ? transaction.description : tag + " - " + transaction.description);
		transactionItem->setText(2, "$" + QString::number(std::abs(transaction.amount), 'f', 2));
		transactionItem->setText(3, transaction.sourceFile);
	}

	for (auto it = sectionItems.begin(); it != sectionItems.end(); ++it) {
		it.value()->setText(2, "$" + QString::number(sectionTotals.value(it.key()), 'f', 2));
	}

	auto* skippedItem = new QTreeWidgetItem(reviewTree);
	skippedItem->setText(0, "Skipped");
	QFont skippedFont = skippedItem->font(0);
	skippedFont.setBold(true);
	for (int column = 0; column < 4; ++column) {
		skippedItem->setFont(column, skippedFont);
	}

	for (const Transaction& transaction : reportSession.transactions()) {
		if (!transaction.skipped) {
			continue;
		}

		auto* transactionItem = new QTreeWidgetItem(skippedItem);
		transactionItem->setText(0, transaction.date.toString("yyyy-MM-dd"));
		transactionItem->setText(1, transaction.description);
		transactionItem->setText(2, "$" + QString::number(std::abs(transaction.amount), 'f', 2));
		transactionItem->setText(3, transaction.sourceFile);
	}

	reviewTree->expandAll();
	for (int column = 0; column < reviewTree->columnCount(); ++column) {
		reviewTree->resizeColumnToContents(column);
	}
}

void MainWindow::addManualTransaction()
{
	QStringList rootNames;

	for (const BudgetSection& root : budgetFormat.roots()) {
		rootNames.append(root.name);
	}

	bool ok = false;
	const QString rootName = QInputDialog::getItem(this, "Add Transaction", "Income or expense:", rootNames, 0, false, &ok);

	if (!ok || rootName.isEmpty()) {
		return;
	}

	const BudgetSection* selectedRoot = nullptr;

	for (const BudgetSection& root : budgetFormat.roots()) {
		if (root.name == rootName) {
			selectedRoot = &root;
			break;
		}
	}

	if (selectedRoot == nullptr || selectedRoot->children.isEmpty()) {
		QMessageBox::warning(this, "No Sections", "Add a section before adding a manual transaction.");
		return;
	}

	QStringList sectionNames;

	for (const BudgetSection& section : selectedRoot->children) {
		sectionNames.append(section.name);
	}

	const QString sectionName = QInputDialog::getItem(this, "Add Transaction", "Section:", sectionNames, 0, false, &ok);

	if (!ok || sectionName.isEmpty()) {
		return;
	}

	const BudgetSection* selectedSection = nullptr;

	for (const BudgetSection& section : selectedRoot->children) {
		if (section.name == sectionName) {
			selectedSection = &section;
			break;
		}
	}

	QStringList tagNames = { "No tag" };

	if (selectedSection != nullptr) {
		for (const BudgetSection& tag : selectedSection->children) {
			tagNames.append(tag.name);
		}
	}

	const QString tagName = QInputDialog::getItem(this, "Add Transaction", "Tag:", tagNames, 0, false, &ok);

	if (!ok || tagName.isEmpty()) {
		return;
	}

	QString sectionPath = rootName + " / " + sectionName;

	if (tagName != "No tag") {
		sectionPath += " / " + tagName;
	}

	const QString dateText = QInputDialog::getText(this, "Add Transaction", "Date (YYYY-MM-DD):", QLineEdit::Normal, QDate::currentDate().toString("yyyy-MM-dd"), &ok).trimmed();

	if (!ok || dateText.isEmpty()) {
		return;
	}

	const QDate date = QDate::fromString(dateText, "yyyy-MM-dd");

	if (!date.isValid()) {
		QMessageBox::warning(this, "Invalid Date", "Use the YYYY-MM-DD date format.");
		return;
	}

	const QString description = QInputDialog::getText(this, "Add Transaction", "Description:", QLineEdit::Normal, "", &ok).trimmed();

	if (!ok || description.isEmpty()) {
		return;
	}

	const double amount = QInputDialog::getDouble(this, "Add Transaction", "Amount:", 0.0, 0.0, 100000000.0, 2, &ok);

	if (!ok) {
		return;
	}

	Transaction transaction;
	transaction.sourceFile = "Manual Entry";
	transaction.date = date;
	transaction.description = description;
	transaction.amount = amount;
	transaction.assignedPath = sectionPath;
	transaction.skipped = false;

	reportSession.addManualTransaction(transaction);
	populateReviewTree();
}

void MainWindow::exportPdf()
{
	const QString filePath = QFileDialog::getSaveFileName(this, "Export PDF", "ExpenseBot Report.pdf", "PDF Files (*.pdf)");

	if (filePath.isEmpty()) {
		return;
	}

	QString error;
	if (!reportExporter.exportPdf(filePath, reportSession, budgetFormat, &error)) {
		QMessageBox::warning(this, "Export Failed", error);
		return;
	}

	QMessageBox::information(this, "Export Complete", "PDF report exported.");
}

void MainWindow::exportXlsx()
{
	const QString filePath = QFileDialog::getSaveFileName(this, "Export XLSX", "ExpenseBot Report.xlsx", "Excel Files (*.xlsx)");

	if (filePath.isEmpty()) {
		return;
	}

	QString error;
	if (!reportExporter.exportXlsx(filePath, reportSession, budgetFormat, &error)) {
		QMessageBox::warning(this, "Export Failed", error);
		return;
	}

	QMessageBox::information(this, "Export Complete", "XLSX report exported.");
}
