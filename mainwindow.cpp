#include "CSVParser.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QTableView>

using namespace mainSpace;
using namespace CSV;
namespace
{
    constexpr int TRANSACTIONS_COL_LEN = 6;
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
    if(!filePath.isEmpty())
    {
        CSVParser csvParser(nullptr);
        QVector<Transaction> transactions = csvParser.ParseCSV(filePath);
        if(!transactions.isEmpty())
        {
            PopulateDataTableWithTransactions(transactions);
        }
    }
}

QString MainWindow::OpenFileDialog()
{
    return QFileDialog::getOpenFileName(nullptr, "Select Bank Record", QDir::homePath(), "CSV Files (*.csv);;All Files (*.*)");
}

void MainWindow::PopulateDataTableWithTransactions(const QVector<Transaction>& transactions)
{
    QStandardItemModel* model = new QStandardItemModel(transactions.length(), TRANSACTIONS_COL_LEN);
    model->setHorizontalHeaderLabels({"Account", "Debit", "Credit", "Balance", "Date", "Description"});
    for (int row = 0; row < transactions.size(); ++row)
    {
        const Transaction& t = transactions[row];
        model->setItem(row, 0, new QStandardItem(t.Account));
        model->setItem(row, 1, new QStandardItem(t.Debit));
        model->setItem(row, 2, new QStandardItem(t.Credit));
        model->setItem(row, 3, new QStandardItem(t.Balance));
        model->setItem(row, 4, new QStandardItem(t.Date));
        model->setItem(row, 5, new QStandardItem(t.Desc));
    }

    ui->DataTableView->setModel(model);
    ui->DataTableView->resizeColumnsToContents();
    ui->DataTableView->resizeRowsToContents();
}
