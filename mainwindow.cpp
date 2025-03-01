#include "BillAdderWidget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QTableView>

#include <iostream>

using namespace mainSpace;
using namespace CSV;
namespace
{
    constexpr int TRANSACTIONS_COL_LEN = 6;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , CSVParserInstance(nullptr)
{
    ui->setupUi(this);
    connect(ui->UploadBankCSVButton, &QPushButton::clicked, this, &MainWindow::OnUploadBankCSVButtonClicked, Qt::UniqueConnection);
    connect(ui->AddBillsButton, &QPushButton::clicked, this, &MainWindow::OnAddBillsButtonClicked, Qt::UniqueConnection);
    // TODO: We will want to query active bills on constuct (or on show breakdown whatever if not loaded) so that we can calculate for that step


    //ReviewBillsButton
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
        LatestTransactions = CSVParserInstance.ParseTransactionCSV(filePath);
        if(!LatestTransactions.isEmpty())
        {
            PopulateDataTableWithTransactions(LatestTransactions);
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
    for(int row = 0; row < transactions.size(); ++row)
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

void MainWindow::OnAddBillsButtonClicked()
{
    Bills::BillAdderWidget* billAdder = new Bills::BillAdderWidget();
    connect(billAdder, &Bills::BillAdderWidget::NotifyBillAdded, this, &MainWindow::HandleBillAdded, Qt::UniqueConnection);
    billAdder->show();
}

void MainWindow::HandleBillAdded(const QString& desc, const QString& ammt)
{
    CSVParserInstance.AddBill(desc, ammt);
    ShowBillsView();
}

void MainWindow::ShowBillsView()
{
    // TODO: SN: Similiar to Transactions, I debate if we need to start caching this, could cache in the parser, so we know it has the last time called values
    QMap<QString, QString> bills = CSVParserInstance.GetAllBills();
    if(!bills.empty())
    {
        QStandardItemModel* model = new QStandardItemModel(bills.size(), 2);
        model->setHorizontalHeaderLabels({"Desc", "Ammnt"});
        int rowCount = 0;
        for(auto it = bills.constBegin(); it != bills.constEnd(); ++it)
        {
            model->setItem(rowCount, 0, new QStandardItem(it.key()));
            model->setItem(rowCount, 1, new QStandardItem(it.value()));
            ++rowCount;
        }
        ui->DataTableView->setModel(model);
        ui->DataTableView->resizeColumnsToContents();
        ui->DataTableView->resizeRowsToContents();
    }
}
