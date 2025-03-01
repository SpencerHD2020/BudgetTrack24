#include "BillAdderWidget.h"
#include "ui_BillAdderWidget.h"

using namespace Bills;

BillAdderWidget::BillAdderWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BillAdderWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->AcceptButton->setDisabled(true); // Disable Button until valid entry
    connect(ui->AmmountTextEdit, &QTextEdit::textChanged, this, &BillAdderWidget::HandleTextEntryChanged, Qt::UniqueConnection);
    connect(ui->DescriptionEdit, &QTextEdit::textChanged, this, &BillAdderWidget::HandleTextEntryChanged, Qt::UniqueConnection);
    connect(ui->AcceptButton, &QPushButton::clicked, this, &BillAdderWidget::HandleAcceptButtonClicked, Qt::UniqueConnection);
}

BillAdderWidget::~BillAdderWidget()
{
    delete ui;
}

void BillAdderWidget::HandleTextEntryChanged()
{
    bool ammountIsNumeric;
    const double ammount = ui->AmmountTextEdit->toPlainText().toDouble(&ammountIsNumeric);
    Q_UNUSED(ammount);
    // Check validity of both entries, if both valid, enable accept button
    ui->AcceptButton->setDisabled(ui->DescriptionEdit->toPlainText().isEmpty() || !ammountIsNumeric);
}

void BillAdderWidget::HandleAcceptButtonClicked()
{
    emit NotifyBillAdded(ui->DescriptionEdit->toPlainText(), ui->AmmountTextEdit->toPlainText());
    close();
}
