#include "CSVParser.h"
#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include <algorithm>


#include <iostream>

using namespace CSV;
namespace
{
    // Debit is subtract, Credit is add
    const QVector<QString> TRANSACTION_COLUMN_NAMES = {"Date", "Description", "Debit/Credit", "Balance" };
    const QString APP_DATA_DIR_NAME = "BudgetTrack";
    const QString BILLS_CSV_NAME = "bills.csv";
    const QString CC_CSV_NAME = "Credit.csv";
    // CSV to store last 100 Transactions
    const QString LEGACY_TRANSACTIONS_CSV_NAME = "LegacyTransactions.csv";
    constexpr int MAX_PERSISTED_TRANSACTIONS = 100;
    const QString TOTALS_CSV_NAME = "Totals.csv";
}



// TODO: SN: Maybe we should store [From Transaction Bank Data] the last 2-3 Transactions
//      in app data so when a new file is uploaded. We can pick up where we left off.

CSVParser::CSVParser(QObject *parent)
    : QObject{parent}
{
    LoadCurrentTotalsFromCSVIfExists();
    LoadTransactionsFromCSVIfExists();

    // Need to Load In Bills and CC Data


    // We should also Load Transactions on construct

    // Tech Debt, a very Hacky way to ensure we have good objects from the jump
    QMap<int, QPair<QString, QString>> bills = GetAllBills();
    QMap<int, QPair<QString, QString>> cc = GetCCData();
    Q_UNUSED(bills);
    Q_UNUSED(cc);
}

QVector<Transaction> CSVParser::HandleNewTransactionCSVAdded(const QString& filePath)
{
    QVector<Transaction> newTransactions = ParseTransactionCSV(filePath); // Gets sorted in accending date order by this method
    if(!newTransactions.empty())
    {
        if(CreateEmptyLegacyTransactionsCSVIfNotExists()) // CSV Existed
        {
            QVector<Transaction> legacyTrans = ParseTransactionCSV(GetLegacyTransactionsCSVPath());
            const int newStartingPoint = newTransactions.indexOf(legacyTrans.last());
            if(-1 != newStartingPoint)
            {
                // Delete everything up to and including this index from new transactions
                newTransactions.remove(0, (newStartingPoint + 1));

                // Combine the 2 vectors, if neccessary to cap at MAX_PERSISTED_TRANSACTIONS entries
                CurrentTransactions.clear();
                CurrentTransactions = legacyTrans;
                CurrentTransactions.append(newTransactions);

                if(CurrentTransactions.size() >= MAX_PERSISTED_TRANSACTIONS)
                {
                    CurrentTransactions.remove(0, (CurrentTransactions.size() - MAX_PERSISTED_TRANSACTIONS));
                }
            }
            else
            {
                // Gap or something???? Just blindly combine them for now - Take below logic maybe
                const int newTransAmmnt = (newTransactions.size() > MAX_PERSISTED_TRANSACTIONS ? MAX_PERSISTED_TRANSACTIONS : newTransactions.size());
                if(legacyTrans.size() >= newTransAmmnt)
                {
                    legacyTrans.remove(0, newTransAmmnt);
                }

                // Make Current Transactions, Legacy Trans + New Trans
                CurrentTransactions.clear();
                CurrentTransactions = legacyTrans;
                CurrentTransactions.append(newTransactions);
            }
        }
        else // This is the first CSV ever uploaded to the application
        {
            if(newTransactions.size() > MAX_PERSISTED_TRANSACTIONS)
            {
                newTransactions.remove(0, (newTransactions.size() - MAX_PERSISTED_TRANSACTIONS));
            }
            CurrentTransactions = newTransactions;
        }
        // Overwrite Legacy Sheet with updated CurrentTransactions Vector
        SaveCurrentTransactionsToCSV();

        // Also need to reconfigure totals at this point
        ReconfigureCurrentTotals();
    }
    return CurrentTransactions;
}

void CSVParser::AddBill(const QString& desc, const QString& ammt)
{
    EnsureAppDatafolderExists();
    QFile billsCSV(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + BILLS_CSV_NAME);
    if(!billsCSV.exists())
    {
        CreateEmptyBillsCSV();
    }
    if(billsCSV.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream stream(&billsCSV);
        stream << desc.trimmed() << "," << ammt.trimmed() << "\n";
        billsCSV.close();
    }
    CurrentBills.insert((CurrentBills.size() + 1), {desc, ammt});
    ReconfigureCurrentTotals();
}

void CSVParser::EnsureAppDatafolderExists()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME;
    QDir appDataDir(path);
    if(!appDataDir.exists())
    {
        appDataDir.mkpath(path);
    }
}

void CSVParser::CreateEmptyBillsCSV()
{
    QFile billsCSV(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + BILLS_CSV_NAME);
    if(billsCSV.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&billsCSV);
        stream << "Desc,Ammt\n";
        billsCSV.close();
    }
}

QMap<int, QPair<QString, QString>> CSVParser::GetAllBills() /* Desc, Ammt */
{
    if(CurrentBills.empty())
    {
        EnsureAppDatafolderExists();
        QFile billsCSV(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + BILLS_CSV_NAME);
        if(billsCSV.exists())
        {
            if(billsCSV.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream in(&billsCSV);
                int lineCount = 0;
                CurrentBills.clear();
                while(!in.atEnd())
                {
                    QStringList lineData = in.readLine().split(",");
                    if(lineCount > 0)
                    {
                        CurrentBills.insert(lineCount, {lineData[0].trimmed(), lineData[1].trimmed()});
                    }
                    ++lineCount;
                }
            }
        }
    }
    return CurrentBills;
}

QMap<int, QPair<QString, QString>> CSVParser::GetCCData()
{
    if(CurrentCCData.empty())
    {
        EnsureAppDatafolderExists();
        QFile ccCSV(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + CC_CSV_NAME);
        if(ccCSV.exists())
        {
            if(ccCSV.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream in(&ccCSV);
                int lineCount = 0;
                CurrentCCData.clear();
                while(!in.atEnd())
                {
                    QStringList lineData = in.readLine().split(",");
                    if(lineCount > 0)
                    {
                        CurrentCCData.insert(lineCount, {lineData[0].trimmed(), lineData[1].trimmed()});
                    }
                    ++lineCount;
                }
            }
        }
    }
    return CurrentCCData;
}

void CSVParser::AddCC(const QString& desc, const QString& ammt)
{
    EnsureAppDatafolderExists();
    QFile ccCSV(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + CC_CSV_NAME);
    if(!ccCSV.exists())
    {
        CreateEmptyCCCSV();
    }
    if(ccCSV.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream stream(&ccCSV);
        stream << desc.trimmed() << "," << ammt.trimmed() << "\n";
        ccCSV.close();
    }
    CurrentCCData.insert((CurrentCCData.size() + 1), {desc, ammt});
    ReconfigureCurrentTotals();
}

void CSVParser::CreateEmptyCCCSV()
{
    QFile ccCSV(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + CC_CSV_NAME);
    if(ccCSV.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&ccCSV);
        stream << "Desc,Ammt\n";
        ccCSV.close();
    }
}

void CSVParser::HandleBillUpdated(const int index, const QString& name, const QString& ammnt)
{
    // Update Local Object
    std::cout << "BILL Updated: " << CurrentBills.value(index).first.toStdString() << " - " << CurrentBills.value(index).second.toStdString() << " .. Changed to: " << name.toStdString() << " - " << ammnt.toStdString() << std::endl;
    CurrentBills[index] = {name, ammnt};

    // Update CSV File - Realistically, it would be best to find the line in CSV and just replace that. But IK these files will never get
    //      that long, so it will be MUCH easier to just wipe the file and rewrite it.

    QFile billsCSV(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + BILLS_CSV_NAME);
    if(billsCSV.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&billsCSV);
        stream << "Desc,Ammt\n";
        for(int i = 1; i <= CurrentBills.size(); ++i)
        {
            stream << CurrentBills.value(i).first.trimmed() << "," << CurrentBills.value(i).second.trimmed() << "\n";
        }
        billsCSV.close();
    }
    ReconfigureCurrentTotals();
}

void CSVParser::HandleCCUpdated(const int index, const QString& name, const QString& ammnt)
{
    // Update Local Object
    std::cout << "CC Updated: " << CurrentCCData.value(index).first.toStdString() << " - " << CurrentCCData.value(index).second.toStdString() << " .. Changed to: " << name.toStdString() << " - " << ammnt.toStdString() << std::endl;
    CurrentCCData[index] = {name, ammnt};

    // Update CSV File - Realistically, it would be best to find the line in CSV and just replace that. But IK these files will never get
    //      that long, so it will be MUCH easier to just wipe the file and rewrite it.

    QFile ccCSV(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + CC_CSV_NAME);
    if(ccCSV.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&ccCSV);
        stream << "Desc,Ammt\n";
        for(int i = 1; i <= CurrentCCData.size(); ++i)
        {
            stream << CurrentCCData.value(i).first.trimmed() << "," << CurrentCCData.value(i).second.trimmed() << "\n";
        }
        ccCSV.close();
    }
    ReconfigureCurrentTotals();
}

void CSVParser::ReconfigureCurrentTotals()
{
    // Take last of CurrentTransactions and take Balance from there - RawTotal
    CurrentTotals.RawTotal = CurrentTransactions.last().Balance;

    // Iterate Bills and find total Bills - CurrentBills QMap<int, QPair<QString, QString>>
    double billTotal = 0;
    for(int billIndex = 0; billIndex < CurrentBills.count(); ++billIndex)
    {
        billTotal += CurrentBills.value(billIndex).second.toDouble();
    }
    CurrentTotals.TotalBills = QString::number(billTotal);

    // Iterate CC and find total debt - QMap<int, QPair<QString, QString>> CurrentCCData;
    double ccTotal = 0;
    for(int ccIndex = 0; ccIndex < CurrentCCData.count(); ++ccIndex)
    {
        ccTotal += CurrentCCData.value(ccIndex).second.toDouble();
    }
    CurrentTotals.TotalDebt = QString::number(ccTotal);

    // Calculate total extra
    CurrentTotals.TotalExtra = QString::number(CurrentTotals.RawTotal.toDouble() - (CurrentTotals.TotalBills.toDouble() + CurrentTotals.TotalDebt.toDouble()));

    // Update totals CSV file - Will need logic to load this in somewhere on boot most likely
    SaveCurrentTotalsToCSV();

    // Notify UI, but UI will NOT cache value, they will just update the UI if it is shown
    emit NotifyTotalsUpdated(CurrentTotals);
}

bool CSVParser::CreateEmptyLegacyTransactionsCSVIfNotExists()
{
    QFile transCSV(GetLegacyTransactionsCSVPath());
    if(!transCSV.exists())
    {
        if(transCSV.open(QIODevice::WriteOnly))
        {
            QTextStream stream(&transCSV);
            stream << "Acct,Debit,Credit,Balance,Date,Desc\n";
            transCSV.close();
        }
        return false;
    }
    else
    {
        return true;
    }
}

void CSVParser::SortTransactions(QVector<Transaction>& transactions)
{
    std::sort(transactions.begin(), transactions.end(),
              [](const Transaction& a, const Transaction& b) {
                  return a.Date < b.Date; // ascending order (oldest first)
              });
}

QVector<Transaction> CSVParser::ParseTransactionCSV(const QString& filePath)
{
    QVector<Transaction> newTransactions;
    QFile csv(filePath);
    if(csv.exists())
    {
        if(csv.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&csv);
            int lineCount = 0;
            QMap<QString, int> columnIndices;
            bool columnverificationComplete = false;

            QTime time(23, 59);
            QDate currentDate(1942, 5, 29);

            while(!in.atEnd())
            {
                //QStringList lineData = in.readLine().split(",");
                QStringList lineData = ParseCSVLine(in.readLine());
                if(lineCount == 0)
                {
                    for(int i = 0; i < lineData.length(); ++i)
                    {
                        columnIndices.insert(lineData[i].trimmed(), i);
                    }
                }
                else
                {
                    // COLUMN VERIFICATION - ONCE PER CSV
                    if(!columnverificationComplete)
                    {
                        QVector<QString> keys = columnIndices.keys().toVector();
                        bool columnVerificationPassed = true;
                        for(const QString& col : TRANSACTION_COLUMN_NAMES)
                        {
                            if(!keys.contains(col))
                            {
                                columnVerificationPassed = false;
                                break;
                            }
                        }
                        if(columnVerificationPassed)
                        {
                            columnverificationComplete = true;
                        }
                        else
                        {
                            // TODO: Emit Signal, maybe do above to point out problem, could miss connect though, print might be better
                            break;
                        }
                    }


                    const QDate newDate = ConvertTBKStringToDateTime(lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[0])].trimmed(), time).date();
                    if(newDate != currentDate)
                    {
                        currentDate = newDate;
                        time = QTime(23, 59);
                    }
                    else
                    {
                        time = time.addSecs(-60); // Go back in time one min
                    }



                    newTransactions.push_back(Transaction(QString("4330007"), /*Acct*/
                                                          lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[2])].trimmed(), /*Delta*/
                                                          lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[3])].trimmed(), /*Balance*/


                                                          // We need some way to give later times to matching dates based on how high they are in the sheet
                                                          // We could start the time at 11:59p and cache its date, on next iteration if same, subtract 10m and then pass that to the convert func

                                                          ConvertTBKStringToDateTime(lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[0])].trimmed(), time), /*Date*/
                                                          lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[1])].trimmed() /*Desc*/
                                                          ));
                }

                ++lineCount;
            }
            SortTransactions(newTransactions);

        }
    }
    csv.close();
    return newTransactions;
}

void CSVParser::SaveCurrentTransactionsToCSV() const
{
    QFile legacyTrans(GetLegacyTransactionsCSVPath());
    legacyTrans.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream out(&legacyTrans);
    out << "Date,Description,Debit/Credit,Balance\n";
    for(const Transaction& tran : CurrentTransactions)
    {
        // Could be issues with Date format and future runs here... Will need to do some testing there
        out << tran.Date.toString() << "," << tran.Desc << "," << tran.Delta << "," << tran.Balance << "\n";
    }
    legacyTrans.close();
}

QString CSVParser::GetLegacyTransactionsCSVPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + LEGACY_TRANSACTIONS_CSV_NAME;
}

QString CSVParser::GetCurrentTotalsCSVPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + TOTALS_CSV_NAME;
}

void CSVParser::SaveCurrentTotalsToCSV()
{
    CreateCurrentTotalsCSVIfNotExists();
    QFile totalsCSV(GetCurrentTotalsCSVPath());
    totalsCSV.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream out(&totalsCSV);
    out << "\"" << CurrentTotals.RawTotal << "\","
        << "\"" << CurrentTotals.TotalBills << "\","
        << "\"" << CurrentTotals.TotalDebt << "\","
        << "\"" << CurrentTotals.TotalExtra << "\"\n";
    out << CurrentTotals.RawTotal << "," << CurrentTotals.TotalBills << "," << CurrentTotals.TotalDebt << "," << CurrentTotals.TotalExtra << "\n";
    totalsCSV.close();
}

void CSVParser::CreateCurrentTotalsCSVIfNotExists()
{
    QFile totalsCSV(GetCurrentTotalsCSVPath());
    if(!totalsCSV.exists())
    {
        if(totalsCSV.open(QIODevice::WriteOnly))
        {
            QTextStream stream(&totalsCSV);
            stream << "RawTotal,TotalBills,TotalDebt,TotalExtra\n";
            totalsCSV.close();
        }
    }
}

void CSVParser::LoadCurrentTotalsFromCSVIfExists()
{
    QFile totalsCSV(GetCurrentTotalsCSVPath());
    if(totalsCSV.exists())
    {
        if(totalsCSV.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&totalsCSV);
            int lineCount = 0;
            while(!in.atEnd())
            {
                QStringList lineData = ParseCSVLine(in.readLine());
                if(lineCount > 0)
                {
                    CurrentTotals = Totals(lineData[0].trimmed(), lineData[1].trimmed(), lineData[2].trimmed(), lineData[3].trimmed());
                    break;
                }
                ++lineCount;
            }
        }
    }
}

void CSVParser::LoadTransactionsFromCSVIfExists()
{
    QFile legacyTrans(GetLegacyTransactionsCSVPath());
    if(legacyTrans.exists())
    {
        if(legacyTrans.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&legacyTrans);
            int lineCount = 0;
            while(!in.atEnd())
            {
                if(lineCount > 0)
                {
                    //QStringList lineData = in.readLine().split(",");
                    QStringList lineData = ParseCSVLine(in.readLine());
                    CurrentTransactions.push_back(Transaction(QString("4330007"), lineData[2].trimmed(), lineData[3].trimmed(), QDateTime::fromString(lineData[0].trimmed()), lineData[1].trimmed()));
                }
                ++lineCount;
            }
        }
    }
}

void CSVParser::HandleTotalsRequested() const
{
    emit NotifyTotalsUpdated(CurrentTotals);
}

QStringList CSVParser::ParseCSVLine(const QString& line)
{
    QStringList result;
    QRegularExpression regex(R"(\"([^\"]*)\"|([^,]+))"); // Matches either "quoted text" OR unquoted text until next comma
    QRegularExpressionMatchIterator it = regex.globalMatch(line);
    while(it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        if(match.captured(1).length())
        {
            result << match.captured(1); // Quoted value
        }
        else
        {
            // Unquoted value
            result << match.captured(2).trimmed();
        }
    }
    return result;
}

QDateTime CSVParser::ConvertTBKStringToDateTime(const QString& date, const QTime& time)
{
    // Format: "Oct 02, 2025"
    QString format = "MMM dd, yyyy";

    QDate dateObj = QDate::fromString(date.trimmed(), format);
    if (!dateObj.isValid())
    {
        qWarning("Failed to parse date: %s", qPrintable(date));
        return QDateTime();
    }

    // Return QDateTime at midnight
    return QDateTime(dateObj, time);
}







