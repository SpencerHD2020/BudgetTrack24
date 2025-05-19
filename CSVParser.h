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


    // Ideally we should have totals stored in a CSV Somewhere to reference past totals, also would like to cache them so we don't have to recalculate them
    //      each time the View comes back up

    // Though we likely will only have 1-2 instances of this struct floating around at a given time (past [when building new] and cached current [write to CSV on calc]])

    struct Totals
    {
        Totals()
        {}

        Totals(QString rawTotal /* Full balance with no deductions */, QString totalBills, QString totalDebt /* CC Total */, QString totalExtra /* rawTotal - (bills + CC) */)
            : RawTotal(rawTotal)
            , TotalBills(totalBills)
            , TotalDebt(totalDebt)
            , TotalExtra(totalExtra)
        {}

        QString RawTotal;
        QString TotalBills;
        QString TotalDebt;
        QString TotalExtra;
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
        void HandleBillUpdated(const int index, const QString& name, const QString& ammnt);

        // CC Getter/Setter
        void AddCC(const QString& desc, const QString& ammt);
        QMap<int, QPair<QString, QString>> GetCCData();
        void HandleCCUpdated(const int index, const QString& name, const QString& ammnt);

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
