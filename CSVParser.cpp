#include "CSVParser.h"
#include <QMap>
#include <QTextStream>
#include <QStringList>

#include <iostream>

using namespace CSV;
namespace
{
// Debit is subtract, Credit is add
const QVector<QString> COLUMN_NAMES = { "Account", "Debit", "Credit", "Balance", "Date", "Description" };
}

CSVParser::CSVParser(QObject *parent)
    : QObject{parent}
{
}
// In Retrospec, making this a public function is probably a better move, but hey
QVector<Transaction> CSVParser::ParseCSV(const QString& filePath)
{

    std::cout << "[CSVParser::ParseCSV]: File Path: " << filePath.toStdString() << std::endl;

    QVector<Transaction> parsedTransactions;
    QFile csv(filePath);
    if(csv.exists())
    {

        std::cout << "[CSVParser::ParseCSV]: CSV Exists" << std::endl;

        if(csv.open(QIODevice::ReadOnly | QIODevice::Text))
        {

            std::cout << "[CSVParser::ParseCSV]: CSV Opened" << std::endl;

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

                        std::cout << "[CSVParser::ParseCSV]: Mapping Column: " << lineData[i].trimmed().toStdString() << " to index: " << i << std::endl;

                        columnIndices.insert(lineData[i].trimmed(), i);
                    }
                }
                else
                {
                    // COLUMN VERIFICATION - ONCE PER CSV
                    if(!columnverificationComplete)
                    {

                        std::cout << "[CSVParser::ParseCSV]: Running Column verification" << std::endl;

                        QVector<QString> keys = columnIndices.keys().toVector();
                        bool columnVerificationPassed = true;
                        for(const QString& col : COLUMN_NAMES)
                        {
                            if(!keys.contains(col))
                            {

                                std::cout << "[CSVParser::ParseCSV]: Column[ " << col.toStdString() << " ] never mapped, verification failed!" << std::endl;

                                columnVerificationPassed = false;
                                break;
                            }
                        }
                        if(columnVerificationPassed)
                        {
                            std::cout << "[CSVParser::ParseCSV]: Column Verification Successful!" << std::endl;

                            columnverificationComplete = true;
                        }
                        else
                        {

                            std::cout << "[CSVParser::ParseCSV]: Column Verification FAILED!" << std::endl;

                            // TODO: Emit Signal, maybe do above to point out problem, could miss connect though, print might be better
                            break;
                        }
                    }

                    std::cout << "[CSVParser::ParseCSV]: Adding Transaction:" << std::endl;
                    std::cout << "[CSVParser::ParseCSV]: Acct: " << lineData[columnIndices.value(COLUMN_NAMES[0])].trimmed().toStdString() << std::endl;
                    std::cout << "[CSVParser::ParseCSV]: Debit: " << lineData[columnIndices.value(COLUMN_NAMES[1])].trimmed().toStdString() << std::endl;
                    std::cout << "[CSVParser::ParseCSV]: Credit: " << lineData[columnIndices.value(COLUMN_NAMES[2])].trimmed().toStdString() << std::endl;
                    std::cout << "[CSVParser::ParseCSV]: Balance: " << lineData[columnIndices.value(COLUMN_NAMES[3])].trimmed().toStdString() << std::endl;
                    std::cout << "[CSVParser::ParseCSV]: Date: " << lineData[columnIndices.value(COLUMN_NAMES[4])].trimmed().toStdString() << std::endl;
                    std::cout << "[CSVParser::ParseCSV]: Desc: " << lineData[columnIndices.value(COLUMN_NAMES[5])].trimmed().toStdString() << std::endl;



                    // Build Transaction Objects and append to QVector
                    parsedTransactions.push_back(Transaction(lineData[columnIndices.value(COLUMN_NAMES[0])].trimmed(), /*Acct*/
                                                             lineData[columnIndices.value(COLUMN_NAMES[1])].trimmed(), /*Debit*/
                                                             lineData[columnIndices.value(COLUMN_NAMES[2])].trimmed(), /*Credit*/
                                                             lineData[columnIndices.value(COLUMN_NAMES[3])].trimmed(), /*Balance*/
                                                             lineData[columnIndices.value(COLUMN_NAMES[4])].trimmed(), /*Date*/
                                                             lineData[columnIndices.value(COLUMN_NAMES[5])].trimmed() /*Desc*/
                                                             ));
                }

                ++lineCount;
                std::cout << "[CSVParser::ParseCSV]: New Line Cnt: " << lineCount << std::endl;
            }
            //TODO: Need to see what debit/credit are like in CSV when not present
        }
    }
    return parsedTransactions;
}
