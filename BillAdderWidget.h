#include <QWidget>

namespace Ui
{
    class BillAdderWidget;
}

namespace Bills
{
    class BillAdderWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit BillAdderWidget(QWidget *parent = nullptr);
        ~BillAdderWidget();

    signals:
        void NotifyBillAdded(const QString& desc, const QString& ammt);

    private slots:
        void HandleTextEntryChanged();
        void HandleAcceptButtonClicked();

    private:
        Ui::BillAdderWidget *ui;
    };
}
