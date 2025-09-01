#include "CSVParser.h"
#include <QDir>
#include <QTextStream>
#include <QStandardPaths>
#include <QStringList>
#include <algorithm>


#include <iostream>

using namespace CSV;
namespace
{
    // Debit is subtract, Credit is add
    const QVector<QString> TRANSACTION_COLUMN_NAMES = { "Account", "Debit", "Credit", "Balance", "Date", "Description" };
    const QString APP_DATA_DIR_NAME = "BudgetTrack";
    const QString BILLS_CSV_NAME = "bills.csv";
    const QString CC_CSV_NAME = "Credit.csv";
    // CSV to store last 100 Transactions
    const QString LEGACY_TRANSACTIONS_CSV_NAME = "LegacyTransactions.csv";
    constexpr int MAX_PERSISTED_TRANSACTIONS = 100;
}



// TODO: SN: Maybe we should store [From Transaction Bank Data] the last 2-3 Transactions
//      in app data so when a new file is uploaded. We can pick up where we left off.

CSVParser::CSVParser(QObject *parent)
    : QObject{parent}
{
}
// TODO: SN: Eventually lets append this too a master CSV so that it persists and can load on boot
//      Would be nice to check if dates or something misaligns and avoid adding to avoid duplicate entries
QVector<Transaction> CSVParser::HandleNewTransactionCSVAdded(const QString& filePath)
{
    QVector<Transaction> newTransactions = ParseTransactionCSV(filePath); // Gets sorted in accending date order by this method
    if(!newTransactions.empty())
    {
        if(CreateEmptyLegacyTransactionsCSVIfNotExists()) // CSV Existed
        {
            QVector<Transaction> legacyTrans = ParseTransactionCSV(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + LEGACY_TRANSACTIONS_CSV_NAME);
            // Count how many new Transactions there are = x

            // Remove oldest x Transactions from Legacy Sheet
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
        else // This is the first CSV ever uploaded to the application
        {
            // Add newest MAX_PERSISTED_TRANSACTIONS (or all if < MAX_PERSISTED_TRANSACTIONS) to Legacy Sheet
            if(newTransactions.size() > MAX_PERSISTED_TRANSACTIONS)
            {
                newTransactions.remove(0, (newTransactions.size() - MAX_PERSISTED_TRANSACTIONS));
                CurrentTransactions = newTransactions;
            }
        }



        // Call Method to overwrite Legacy Sheet with updated CurrentTransactions Vector

        // Also need to reconfigure totals at this point
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
}

void CSVParser::ReconfigureCurrentTotals()
{
    // Fetch old totals - Seperate method - needs to check if they exist? If not, we can just hard code what totals to use to grab inital


    // TODO: SN: Need to add caching of last 4-5 Transactions, find that start point and add any new transactions there that do NOT exist within those 4-5
    // Therefore, this does NOT need to be done here

    // Iterate Transactions and find new raw Total

    // Iterate Bills and find total Bills

    // Iterate CC and find total debt

    // Calculate total extra

    // Update totals CSV file

    // DONT notify UI, we do not want to double cache value, so when that view is going to be shown they can request the Totals struct from here
}

bool CSVParser::CreateEmptyLegacyTransactionsCSVIfNotExists()
{
    QFile transCSV(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + LEGACY_TRANSACTIONS_CSV_NAME);
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

            while(!in.atEnd())
            {
                QStringList lineData = in.readLine().split(",");
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
                    newTransactions.push_back(Transaction(lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[0])].trimmed(), /*Acct*/
                                                                                                                                lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[1])].trimmed(), /*Debit*/
                                                          lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[2])].trimmed(), /*Credit*/
                                                          lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[3])].trimmed(), /*Balance*/
                                                          QDateTime::fromString(lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[4])].trimmed()), /*Date*/
                                                          lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[5])].trimmed() /*Desc*/
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


