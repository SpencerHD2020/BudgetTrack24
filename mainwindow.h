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

    private:
        QString OpenFileDialog();
        void PopulateDataTableWithTransactions(const QVector<CSV::Transaction>& transactions);

        Ui::MainWindow *ui;
    };
}
