#include "CSVParser.h"
#include <QTextStream>
#include <QStringList>

using namespace CSV;
namespace
{
// Debit is subtract, Credit is add
const QVector<QString> COLUMN_NAMES = { "Account", "Chk Ref", "Debit", "Credit", "Balance", "Date", "Description" };
}

CSVParser::CSVParser(const QString csvPath, QObject *parent)
    : QObject{parent}
{
    QFile csv(csvPath);
    if(csv.exists())
    {
        ParseCSV(csv);
    }
}

void CSVParser::ParseCSV(QFile& file)
{
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while(!in.atEnd())
        {
            // Check if first loop, get column indices and save above

            // Ensuring not in first loop
                // Build Transaction Objects and append to QVector
                // Once complete, emit
                // Make a Requestor from MainWindow since this happens on construct and could finish before connect

            //TODO: Need to see what debit/credit are like in CSV when not present
        }
    }
}
