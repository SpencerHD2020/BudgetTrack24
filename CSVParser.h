#include <QFile>
#include <QObject>

namespace CSV
{

    struct Transaction
    {
        Transaction()
        {}

        Transaction(QString account, QString debit, QString credit, QString balance, QString date, QString desc)
            : Account(account)
            , Debit(debit)
            , Credit(credit)
            , Balance(balance)
            , Date(date)
            , Desc(desc)
        {}

        QString Account;
        QString Debit;
        QString Credit;
        QString Balance;
        QString Date;
        QString Desc;
    };

    class CSVParser : public QObject
    {
        Q_OBJECT
    public:
        explicit CSVParser(QObject *parent = nullptr);
        QVector<Transaction> ParseTransactionCSV(const QString& filePath);
        void AddBill(const QString& desc, const QString& ammt);
        QMap<QString, QString> GetAllBills();

    private:
        void EnsureAppDatafolderExists();
        void CreateEmptyBillsCSV();

        QVector<Transaction> Transactions;
    };
}
