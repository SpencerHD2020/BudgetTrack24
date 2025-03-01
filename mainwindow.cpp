#include "CSVParser.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>

#include <iostream>

using namespace mainSpace;
using namespace CSV;
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

void MainWindow::OnUploadBankCSVButtonClicked()
{
    QString filePath = OpenFileDialog();
    std::cout << filePath.toStdString() << std::endl;
    if(!filePath.isEmpty())
    {
        CSVParser csvParser(nullptr);
        QVector<Transaction> transactions = csvParser.ParseCSV(filePath);
        if(!transactions.isEmpty())
        {
            // TODO: Fill UI Table
            for(const Transaction& trans : transactions)
            {
                qDebug() << trans.Desc;

                std::cout << trans.Desc.toStdString() << std::endl;
            }
        }
    }
}

QString MainWindow::OpenFileDialog()
{
    std::cout << "HI" << std::endl;

    return QFileDialog::getOpenFileName(nullptr, "Select Bank Record", QDir::homePath(), "CSV Files (*.csv);;All Files (*.*)");
}
