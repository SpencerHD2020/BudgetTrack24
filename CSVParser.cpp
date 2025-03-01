#include "CSVParser.h"
#include <QMap>
#include <QTextStream>
#include <QStringList>

using namespace CSV;
namespace
{
// Debit is subtract, Credit is add
const QVector<QString> COLUMN_NAMES = { "Account", "Chk Ref", "Debit", "Credit", "Balance", "Date", "Description" };
}

CSVParser::CSVParser(QObject *parent)
    : QObject{parent}
{
}
// In Retrospec, making this a public function is probably a better move, but hey
QVector<Transaction> CSVParser::ParseCSV(const QString& filePath)
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
                        for(const QString& col : COLUMN_NAMES)
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
                    parsedTransactions.push_back(Transaction(lineData[columnIndices.value(COLUMN_NAMES[0])].trimmed(), /*Acct*/
                                                                                                                       lineData[columnIndices.value(COLUMN_NAMES[2])].trimmed(), /*Debit*/
                                                             lineData[columnIndices.value(COLUMN_NAMES[3])].trimmed(), /*Credit*/
                                                             lineData[columnIndices.value(COLUMN_NAMES[4])].trimmed(), /*Balance*/
                                                             lineData[columnIndices.value(COLUMN_NAMES[5])].trimmed(), /*Date*/
                                                             lineData[columnIndices.value(COLUMN_NAMES[6])].trimmed() /*Desc*/
                                                             ));
                }

                ++lineCount;
            }
            //TODO: Need to see what debit/credit are like in CSV when not present
        }
    }
    return parsedTransactions;
}
