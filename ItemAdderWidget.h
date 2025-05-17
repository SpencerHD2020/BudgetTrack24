#include <QWidget>

namespace Ui
{
    class ItemAdderWidget;
}

namespace ItemAdder
{
    class ItemAdderWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit ItemAdderWidget(const QString& dialogName, QWidget *parent = nullptr);
        ~ItemAdderWidget();

    signals:
        void NotifyItemAdded(const QString& desc, const QString& ammt);

    private slots:
        void HandleTextEntryChanged();
        void HandleAcceptButtonClicked();

    private:
        Ui::ItemAdderWidget *ui;
    };
}
