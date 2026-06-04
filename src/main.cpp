#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

        auto* window = new QMainWindow();
        window->setWindowTitle("ExpenseBot");
        window->resize(1000, 700);

        auto* central = new QWidget(window);
        auto* layout = new QVBoxLayout(central);

        auto* title = new QLabel("ExpenseBot");
        auto* createReportButton = new QPushButton("Create Report");
        auto* editFormatButton = new QPushButton("Edit Format");

        layout->addWidget(title);
        layout->addWidget(createReportButton);
        layout->addWidget(editFormatButton);
        layout->addStretch();

        window->setCentralWidget(central);
        window->show();

        return app.exec();
}