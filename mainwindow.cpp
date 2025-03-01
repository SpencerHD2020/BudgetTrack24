#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

using namespace mainSpace;
namespace
{

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->UploadBankCSVButton, &QPushButton::clicked, this, &MainWindow::OnUploadBankCSVButtonClicked, Qt::UniqueConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ParseCSV(const QString& filePath)
{
    Q_UNUSED(filePath);
}

void MainWindow::OnUploadBankCSVButtonClicked()
{
    QString filePath = OpenFileDialog();
    if(!filePath.isEmpty())
    {
        ParseCSV(filePath);
    }
}

QString MainWindow::OpenFileDialog()
{
    return QFileDialog::getOpenFileName(nullptr, "Select Bank Record", QDir::homePath(), "CSV Files (*.csv);;All Files (*.*)");
}
