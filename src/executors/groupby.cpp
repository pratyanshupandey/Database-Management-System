#include "global.h"

/**
 * @brief 
 * SYNTAX: <new_table> <- GROUP BY <grouping_attribute> FROM <table_name> RETURN MAX|MIN|SUM|AVG(<attribute>)
 */

bool syntacticParseGROUPBY()
{
    logger.log("syntacticParseGROUPBY");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[3] != "BY" || tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "RETURN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = GROUPBY;
    parsedQuery.groupRelationName = tokenizedQuery[6];
    parsedQuery.groupResultRelationName = tokenizedQuery[0];
    parsedQuery.groupGroupingAttribute = tokenizedQuery[4];

    string opstring = tokenizedQuery[8];

    if (opstring.back() != ')' || opstring.size() < 6)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    } 
    else
    {
        parsedQuery.groupOperatedAttribute = "";
        for (uint i = 4; i < opstring.size() - 1; i++)
            parsedQuery.groupOperatedAttribute += opstring[i];
    }

    if (opstring.compare(0, 4, "MAX(") == 0)
        parsedQuery.groupOperation = "MAX";
    else if (opstring.compare(0, 4, "MIN(") == 0)
        parsedQuery.groupOperation = "MIN";
    else if (opstring.compare(0, 4, "SUM(") == 0)
        parsedQuery.groupOperation = "SUM";
    else if (opstring.compare(0, 4, "AVG(") == 0)
        parsedQuery.groupOperation = "AVG";
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseGROUPBY()
{
    logger.log("semanticParseGROUPBY");

    if (tableCatalogue.isTable(parsedQuery.groupResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupGroupingAttribute, parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Grouping Column doesn't exist in relation" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupOperatedAttribute, parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Operated Column doesn't exist in relation" << endl;
        return false;
    }

    return true;
}


void executeGROUPBY()
{
    logger.log("executeGROUPBY");
    Table table = *tableCatalogue.getTable(parsedQuery.groupRelationName);

    vector <string> result_columns;
    result_columns.push_back(parsedQuery.groupGroupingAttribute);
    result_columns.push_back(parsedQuery.groupOperation + parsedQuery.groupOperatedAttribute);
    Table *resultTable = new Table(parsedQuery.groupResultRelationName);
    resultTable->sourceFileName = "";
    resultTable->columns = result_columns;
    resultTable->columnCount = result_columns.size();
    resultTable->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * resultTable->columnCount));

    int groupingColumnIndex = table.getColumnIndex(parsedQuery.groupGroupingAttribute);
    int operatingColumnIndex = table.getColumnIndex(parsedQuery.groupOperatedAttribute);

    vector <int> row;
    map <int, pair<int, int>> result;
    Cursor cursor = table.getCursor();

    while(true)
    {
        row = cursor.getNext();
        if (row.empty())
            break;
        if (result.find(row[groupingColumnIndex])  == result.end())
            result[row[groupingColumnIndex]] = pair <int,int>(row[operatingColumnIndex], 1);
        else
        {
            auto updated = result[row[groupingColumnIndex]];

            if (parsedQuery.groupOperation == "MAX")
                updated.first = max(updated.first, row[operatingColumnIndex]);
            else if (parsedQuery.groupOperation == "MIN")
                updated.first = min(updated.first, row[operatingColumnIndex]);
            else if (parsedQuery.groupOperation == "SUM")
                updated.first += row[operatingColumnIndex];
            else if (parsedQuery.groupOperation == "AVG")
                updated.first += row[operatingColumnIndex];            
            updated.second++;
            result[row[groupingColumnIndex]] = updated;
        }
    }

    vector <vector <int>> result_rows;

    for (auto it = result.begin(); it != result.end(); it++)
    {
        vector <int> new_row;
        new_row.push_back(it->first);

        if (parsedQuery.groupOperation == "MAX")
            new_row.push_back(it->second.first);
        else if (parsedQuery.groupOperation == "MIN")
            new_row.push_back(it->second.first);
        else if (parsedQuery.groupOperation == "SUM")
            new_row.push_back(it->second.first);
        else if (parsedQuery.groupOperation == "AVG")
            new_row.push_back((it->second.first)/(it->second.second));


        result_rows.push_back(new_row);
        resultTable->rowCount++;

        if (result_rows.size() == resultTable->maxRowsPerBlock)
        {
            bufferManager.writePage(resultTable->tableName, resultTable->blockCount, result_rows, result_rows.size());
            resultTable->rowsPerBlockCount.push_back(result_rows.size());
            resultTable->blockCount++;
            result_rows.clear();
        }
    }
    if (!result_rows.empty())
    {
        bufferManager.writePage(resultTable->tableName, resultTable->blockCount, result_rows, result_rows.size());
        resultTable->rowsPerBlockCount.push_back(result_rows.size());
        resultTable->blockCount++;
        result_rows.clear();
    }

    tableCatalogue.insertTable(resultTable);

    cout << "\nNo of Block Accesses = " << endl;
    return;
}