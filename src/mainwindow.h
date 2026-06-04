#pragma once

#include <QMainWindow>

class QPushButton;

class MainWindow : public QMainWindow
{
public:
	explicit MainWindow(QWidget* parent = nullptr);
private:
	void buildLandingPage();

	QPushButton* createReportButton = nullptr;
	QPushButton* editFormatButton = nullptr;
};