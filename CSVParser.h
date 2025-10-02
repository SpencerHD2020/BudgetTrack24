#include <QFile>
#include <QMap>
#include <QPair>
#include <QObject>
#include <QDateTime>

namespace CSV
{

    struct Transaction
    {
        Transaction()
        {}

        Transaction(QString account, QString delta, QString balance, QDateTime date, QString desc)
            : Account(account)
            , Delta(delta)
            , Balance(balance)
            , Date(date)
            , Desc(desc)
        {}

        QString Account;
        QString Delta;
        QString Balance;
        QDateTime Date;
        QString Desc;

        bool operator==(const Transaction& other) const
        {
            return Account == other.Account &&
                   Delta == other.Delta &&
                   Balance == other.Balance &&
                   Date == other.Date &&
                   Desc == other.Desc;
        }
        bool operator!=(const Transaction& other) const
        {
            return !(*this == other);
        }
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
        QVector<Transaction> HandleNewTransactionCSVAdded(const QString& filePath);

        // Bill Getter/Setter
        void AddBill(const QString& desc, const QString& ammt);
        QMap<int, QPair<QString, QString>> GetAllBills();
        void HandleBillUpdated(const int index, const QString& name, const QString& ammnt);

        // CC Getter/Setter
        void AddCC(const QString& desc, const QString& ammt);
        QMap<int, QPair<QString, QString>> GetCCData();
        void HandleCCUpdated(const int index, const QString& name, const QString& ammnt);
        void HandleTotalsRequested() const;

    signals:
        void NotifyTotalsUpdated(const Totals& totals) const;

    private:
        void EnsureAppDatafolderExists();
        void CreateEmptyBillsCSV();
        void CreateEmptyCCCSV();
        bool CreateEmptyLegacyTransactionsCSVIfNotExists();

        // As this method gets built out, consider, would it be worth passing an enum to this describing what changed? Then we do not have to fully reconfigure everything?
        //      We MAY have to, because Transaction data is going to be tricky, kind of depends how we do it. IG since Transactions are not all long term stored, we could assume that CurrentTransactions
        //      is always new, but we may need to add a flag to Transaction struct (added to total) or something like that. But more realistically, it will just be prior total - all the transactions in that struct
        //      How do we handle initial run? Could prolly be abit hacked since just me using this

        void ReconfigureCurrentTotals();
        void SortTransactions(QVector<Transaction>& transactions);
        QVector<Transaction> ParseTransactionCSV(const QString& filePath);
        void SaveCurrentTransactionsToCSV() const;
        QString GetLegacyTransactionsCSVPath() const;
        void SaveCurrentTotalsToCSV();
        QString GetCurrentTotalsCSVPath() const;
        void CreateCurrentTotalsCSVIfNotExists();
        void LoadCurrentTotalsFromCSVIfExists();
        void LoadTransactionsFromCSVIfExists();
        QStringList ParseCSVLine(const QString& line);
        QDateTime ConvertTBKStringToDateTime(const QString& date, const QTime& time);

        QVector<Transaction> CurrentTransactions;
        QMap<int, QPair<QString, QString>> CurrentBills;
        QMap<int, QPair<QString, QString>> CurrentCCData;
        Totals CurrentTotals;
    };
}
