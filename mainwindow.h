#include "CSVParser.h"
#include <QMainWindow>

namespace Ui
{
    class MainWindow;
}

namespace CSV
{
    struct Transaction;
}

namespace mainSpace
{
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();


    private slots:
        void OnUploadBankCSVButtonClicked();
        void OnAddBillsButtonClicked();
        void HandleBillAdded(const QString& desc, const QString& ammt);
        void ShowBillsView();

        // Credit Card SLOTS
        void OnAddCCButtonClicked();
        void HandleCCAdded(const QString& cardName, const QString& owedAmmt);
        void ShowCCView();

    private:
        QString OpenFileDialog();
        void PopulateDataTableWithTransactions(const QVector<CSV::Transaction>& transactions);

        Ui::MainWindow *ui;
        QVector<CSV::Transaction> LatestTransactions;
        CSV::CSVParser CSVParserInstance;
    };
}
