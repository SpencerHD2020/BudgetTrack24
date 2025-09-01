#include "ItemAdderWidget.h"
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
    , ActivetableView(CurrentDataView_E::NONE)
{
    ui->setupUi(this);
    connect(ui->UploadBankCSVButton, &QPushButton::clicked, this, &MainWindow::OnUploadBankCSVButtonClicked, Qt::UniqueConnection);
    connect(ui->AddBillsButton, &QPushButton::clicked, this, &MainWindow::OnAddBillsButtonClicked, Qt::UniqueConnection);
    connect(ui->ReviewBillsButton, &QPushButton::clicked, this, &MainWindow::ShowBillsView, Qt::UniqueConnection);
    // TODO: We will want to query active bills on constuct (or on show breakdown whatever if not loaded) so that we can calculate for that step
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
        LatestTransactions = CSVParserInstance.HandleNewTransactionCSVAdded(filePath);
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
    ActivetableView = CurrentDataView_E::TRANSACTIONS;
}

void MainWindow::OnAddBillsButtonClicked()
{
    ItemAdder::ItemAdderWidget* billAdder = new ItemAdder::ItemAdderWidget("Add Bill");
    connect(billAdder, &ItemAdder::ItemAdderWidget::NotifyItemAdded, this, &MainWindow::HandleBillAdded, Qt::UniqueConnection);
    billAdder->show();
}

void MainWindow::HandleBillAdded(const QString& desc, const QString& ammt)
{
    CSVParserInstance.AddBill(desc, ammt);
    ShowBillsView();
}

void MainWindow::ShowBillsView()
{
    QMap<int, QPair<QString, QString>> bills = CSVParserInstance.GetAllBills();
    if(!bills.empty())
    {
        QStandardItemModel* model = new QStandardItemModel(bills.size(), 2);
        model->setHorizontalHeaderLabels({"Desc", "Ammnt"});
        int rowCount = 0;
        for(int i = 1; i <= bills.size(); ++i)
        {
            model->setItem(rowCount, 0, new QStandardItem(bills.value(i).first));
            model->setItem(rowCount, 1, new QStandardItem(bills.value(i).second));
            ++rowCount;
        }
        ui->DataTableView->setModel(model);
        ui->DataTableView->resizeColumnsToContents();
        ui->DataTableView->resizeRowsToContents();
        connect(model, &QStandardItemModel::dataChanged, this, &MainWindow::HandleTableDataChanged, Qt::UniqueConnection);
        ActivetableView = CurrentDataView_E::BILLS;
    }
}

// TODO: Connect to main table on change and track what is currently shown, for bills and credit cards, allow edits to be made right there
void MainWindow::HandleTableDataChanged(const QModelIndex& topLeft, const QModelIndex&, const QList<int>&)
{
    const int changedRow = topLeft.row();
    const QAbstractItemModel* model = topLeft.model();

    QVariant column0Data = model->data(model->index(changedRow, 0));
    QVariant column1Data = model->data(model->index(changedRow, 1));

    if(CurrentDataView_E::BILLS == ActivetableView)
    {
        CSVParserInstance.HandleBillUpdated((changedRow + 1), column0Data.toString(), column1Data.toString());
    }
    else if(CurrentDataView_E::CREDIT == ActivetableView)
    {
        CSVParserInstance.HandleCCUpdated((changedRow + 1), column0Data.toString(), column1Data.toString());
    }
}

void MainWindow::OnAddCCButtonClicked()
{
    ItemAdder::ItemAdderWidget* cardAdder = new ItemAdder::ItemAdderWidget("Add Credit Card");
    connect(cardAdder, &ItemAdder::ItemAdderWidget::NotifyItemAdded, this, &MainWindow::HandleCCAdded, Qt::UniqueConnection);
    cardAdder->show();
}

void MainWindow::HandleCCAdded(const QString& cardName, const QString& owedAmmt)
{
    CSVParserInstance.AddCC(cardName, owedAmmt);
    ShowCCView();
}

void MainWindow::ShowCCView()
{
    QMap<int, QPair<QString, QString>> cardData = CSVParserInstance.GetCCData();
    if(!cardData.empty())
    {
        QStandardItemModel* model = new QStandardItemModel(cardData.size(), 2);
        model->setHorizontalHeaderLabels({"Card", "OwedAmmnt"});
        int rowCount = 0;
        for(int i = 1; i <= cardData.size(); ++i)
        {
            model->setItem(rowCount, 0, new QStandardItem(cardData.value(i).first));
            model->setItem(rowCount, 1, new QStandardItem(cardData.value(i).second));
            ++rowCount;
        }
        ui->DataTableView->setModel(model);
        ui->DataTableView->resizeColumnsToContents();
        ui->DataTableView->resizeRowsToContents();
        connect(model, &QStandardItemModel::dataChanged, this, &MainWindow::HandleTableDataChanged, Qt::UniqueConnection);
        ActivetableView = CurrentDataView_E::CREDIT;
    }
}
