#include "CSVParser.h"
#include <QMainWindow>

namespace Ui
{
    class MainWindow;
}

namespace mainSpace
{

    enum class CurrentDataView_E
    {
        NONE,
        TRANSACTIONS,
        BILLS,
        CREDIT,
        TOTALS
    };

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
        void HandleTableDataChanged(const QModelIndex& topLeft, const QModelIndex&, const QList<int>&);

        // Credit Card SLOTS
        void OnAddCCButtonClicked();
        void HandleCCAdded(const QString& cardName, const QString& owedAmmt);
        void ShowCCView();
        void HandleTotalsUpdated(const CSV::Totals& totals);
        void ShowTotalsView();

    private:
        QString OpenFileDialog();
        void PopulateDataTableWithTransactions(const QVector<CSV::Transaction>& transactions);

        Ui::MainWindow *ui;
        QVector<CSV::Transaction> LatestTransactions;
        CSV::CSVParser CSVParserInstance;
        CurrentDataView_E ActivetableView;
    };
}
