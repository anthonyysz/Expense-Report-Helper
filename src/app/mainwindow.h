#pragma once

#include "budgetformat.h"
#include "pdftransactionreader.h"
#include "reportexporter.h"
#include "reportsession.h"

#include <QMainWindow>
#include <QStringList>

class QLabel;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QTreeWidget;
class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
public:
	explicit MainWindow(QWidget* parent = nullptr);

protected:
	void keyPressEvent(QKeyEvent* event) override;

private:
	void buildLandingPage();
	void buildFormatPage();
	void buildReportPage();
	void buildSortPage();
	void buildReviewPage();

	void showLandingPage();
	void showFormatPage();
	void showReportPage();
	void showSortPage();
	void showReviewPage();

	QString formatFilePath() const;
	void saveBudgetFormat();

	void populateFormatTree();
	void addSectionToTree(QTreeWidgetItem* parentItem, const BudgetSection& section, const QVector<int>& path);
	BudgetSection* sectionForPath(const QVector<int>& path);
	BudgetSection* sectionForItem(QTreeWidgetItem* item);
	QVector<BudgetSection>* siblingListForItem(QTreeWidgetItem* item);
	int depthForItem(QTreeWidgetItem* item) const;

	void addSection();
	void addBabySection();
	void renameSection();
	void removeSection();

	void choosePdfFiles();
	void clearPdfFiles();
	void startReport();
	void updateSortPage();
	void populateSortingTree();
	void addSortingSectionToTree(QTreeWidgetItem* parentItem, const BudgetSection& section, const QString& path, const QVector<int>& indexPath);
	QString selectedSortingPath() const;
	void assignSelectedTransaction();
	void skipTransaction();
	void undoAssignment();
	void addSectionDuringSorting();
	void addBabySectionDuringSorting();
	void renameSectionDuringSorting();
	void removeSectionDuringSorting();
	void populateReviewTree();
	void addManualTransaction();
	void exportPdf();
	void exportXlsx();

	BudgetFormat budgetFormat;
	PdfTransactionReader pdfReader;
	ReportSession reportSession;
	ReportExporter reportExporter;
	QStringList selectedPdfFiles;

	QStackedWidget* pageStack = nullptr;

	QPushButton* createReportButton = nullptr;
	QPushButton* editFormatButton = nullptr;

	QPushButton* formatBackButton = nullptr;
	QPushButton* addSectionButton = nullptr;
	QPushButton* addBabySectionButton = nullptr;
	QPushButton* renameSectionButton = nullptr;
	QPushButton* removeSectionButton = nullptr;
	QTreeWidget* formatTree = nullptr;

	QListWidget* pdfFileList = nullptr;
	QPushButton* addPdfFilesButton = nullptr;
	QPushButton* clearPdfFilesButton = nullptr;
	QPushButton* startReportButton = nullptr;
	QPushButton* reportBackButton = nullptr;

	QLabel* progressLabel = nullptr;
	QLabel* sourceFileLabel = nullptr;
	QLabel* dateLabel = nullptr;
	QLabel* descriptionLabel = nullptr;
	QLabel* amountLabel = nullptr;
	QTreeWidget* sortingTree = nullptr;
	QPushButton* nextButton = nullptr;
	QPushButton* skipButton = nullptr;
	QPushButton* undoButton = nullptr;
	QPushButton* sortAddSectionButton = nullptr;
	QPushButton* sortAddBabySectionButton = nullptr;
	QPushButton* sortRenameSectionButton = nullptr;
	QPushButton* sortRemoveSectionButton = nullptr;
	QPushButton* reviewButton = nullptr;
	QPushButton* sortBackButton = nullptr;

	QTreeWidget* reviewTree = nullptr;
	QPushButton* addManualTransactionButton = nullptr;
	QPushButton* exportPdfButton = nullptr;
	QPushButton* exportXlsxButton = nullptr;
	QPushButton* reviewBackButton = nullptr;
};
