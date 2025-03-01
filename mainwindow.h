#include <QMainWindow>

namespace Ui
{
    class MainWindow;
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

        Ui::MainWindow *ui;
    };
}
