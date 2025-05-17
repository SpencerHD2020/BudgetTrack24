#include <QFile>
#include <QMap>
#include <QPair>
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

        // Bill Getter/Setter
        void AddBill(const QString& desc, const QString& ammt);
        QMap<int, QPair<QString, QString>> GetAllBills();

        // CC Getter/Setter
        void AddCC(const QString& desc, const QString& ammt);
        QMap<int, QPair<QString, QString>> GetCCData();

    private:
        void EnsureAppDatafolderExists();
        void CreateEmptyBillsCSV();
        void CreateEmptyCCCSV();

        QVector<Transaction> CurrentTransactions;


        // TODO: SN: Need to make these QMap<int, QPair<QString, QString>> where int is index so that we can track what changed on change
        QMap<int, QPair<QString, QString>> CurrentBills;
        QMap<int, QPair<QString, QString>> CurrentCCData;
    };
}
