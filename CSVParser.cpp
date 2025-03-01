#include "CSVParser.h"
#include <QDir>
#include <QMap>
#include <QTextStream>
#include <QStandardPaths>
#include <QStringList>

using namespace CSV;
namespace
{
    // Debit is subtract, Credit is add
    const QVector<QString> TRANSACTION_COLUMN_NAMES = { "Account", "Debit", "Credit", "Balance", "Date", "Description" };
    const QString APP_DATA_DIR_NAME = "BudgetTrack";
    const QString BILLS_CSV_NAME = "bills.csv";
}

CSVParser::CSVParser(QObject *parent)
    : QObject{parent}
{
}
// In Retrospec, making this a public function is probably a better move, but hey
QVector<Transaction> CSVParser::ParseTransactionCSV(const QString& filePath)
{
    QVector<Transaction> parsedTransactions;
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
                    // Build Transaction Objects and append to QVector
                    parsedTransactions.push_back(Transaction(lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[0])].trimmed(), /*Acct*/
                                                             lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[1])].trimmed(), /*Debit*/
                                                             lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[2])].trimmed(), /*Credit*/
                                                             lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[3])].trimmed(), /*Balance*/
                                                             lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[4])].trimmed(), /*Date*/
                                                             lineData[columnIndices.value(TRANSACTION_COLUMN_NAMES[5])].trimmed() /*Desc*/
                                                             ));
                }

                ++lineCount;
            }
            //TODO: Need to see what debit/credit are like in CSV when not present
        }
    }
    csv.close();
    return parsedTransactions;
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

QMap<QString, QString> CSVParser::GetAllBills() /* Desc, Ammt */
{
    QMap<QString, QString> bills;
    EnsureAppDatafolderExists();
    QFile billsCSV(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + APP_DATA_DIR_NAME + QDir::separator() + BILLS_CSV_NAME);
    if(billsCSV.exists())
    {
        if(billsCSV.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&billsCSV);
            int lineCount = 0;
            while(!in.atEnd())
            {
                QStringList lineData = in.readLine().split(",");
                if(lineCount > 0)
                {
                    bills.insert(lineData[0].trimmed(), lineData[1].trimmed());
                }
                ++lineCount;
            }
        }
    }
    return bills;
}
