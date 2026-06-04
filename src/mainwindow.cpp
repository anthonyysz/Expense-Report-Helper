#include "mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
	setWindowTitle("ExpenseBot");
	resize(1000, 700);

	buildLandingPage();
}

void MainWindow::buildLandingPage() {
	auto* centralWidget = new QWidget(this);
	auto* layout = new QVBoxLayout(centralWidget);

	auto* titleLabel = new QLabel("ExpenseBot", centralWidget);
	createReportButton = new QPushButton("Create Report", centralWidget);
	editFormatButton = new QPushButton("Edit Format", centralWidget);

	layout->addWidget(titleLabel);
	layout->addWidget(createReportButton);
	layout->addWidget(editFormatButton);
	layout->addStretch();

	setCentralWidget(centralWidget);
}