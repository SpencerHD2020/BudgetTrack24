#include "ItemAdderWidget.h"
#include "ui_ItemAdderWidget.h"

using namespace ItemAdder;

ItemAdderWidget::ItemAdderWidget(const QString& dialogName, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ItemAdderWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(dialogName);
    ui->AcceptButton->setDisabled(true); // Disable Button until valid entry
    connect(ui->AmmountTextEdit, &QTextEdit::textChanged, this, &ItemAdderWidget::HandleTextEntryChanged, Qt::UniqueConnection);
    connect(ui->DescriptionEdit, &QTextEdit::textChanged, this, &ItemAdderWidget::HandleTextEntryChanged, Qt::UniqueConnection);
    connect(ui->AcceptButton, &QPushButton::clicked, this, &ItemAdderWidget::HandleAcceptButtonClicked, Qt::UniqueConnection);
}

ItemAdderWidget::~ItemAdderWidget()
{
    delete ui;
}

void ItemAdderWidget::HandleTextEntryChanged()
{
    bool ammountIsNumeric;
    const double ammount = ui->AmmountTextEdit->toPlainText().toDouble(&ammountIsNumeric);
    Q_UNUSED(ammount);
    // Check validity of both entries, if both valid, enable accept button
    ui->AcceptButton->setDisabled(ui->DescriptionEdit->toPlainText().isEmpty() || !ammountIsNumeric);
}

void ItemAdderWidget::HandleAcceptButtonClicked()
{
    emit NotifyItemAdded(ui->DescriptionEdit->toPlainText(), ui->AmmountTextEdit->toPlainText());
    close();
}
