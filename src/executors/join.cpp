#include "global.h"
/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 * NEW_SYNTAX: <new_relation_name> <- JOIN USING NESTED <table1>, <table2> ON <column1> <bin_op> <column2> BUFFER <buffer_size>
 * NEW_SYNTAX: <new_relation_name> <- JOIN USING PARTHASH <table1>, <table2> ON <column1> <bin_op> <column2> BUFFER <buffer_size>
 */
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 13 || tokenizedQuery[3] != "USING" || !(tokenizedQuery[4] != "NESTED" || tokenizedQuery[4] != "PARTHASH") || tokenizedQuery[7] != "ON" || tokenizedQuery[11] != "BUFFER")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[5];
    parsedQuery.joinSecondRelationName = tokenizedQuery[6];
    parsedQuery.joinFirstColumnName = tokenizedQuery[8];
    parsedQuery.joinSecondColumnName = tokenizedQuery[10];
    parsedQuery.joinAlgorithm = tokenizedQuery[4];

    string bufferString = tokenizedQuery[12];
    for (uint i = 0; i < bufferString.size(); i++)
        if(!isdigit(bufferString[i]))
        {
        cout << "SYNTAX ERROR" << endl;
        return false;
        }

    parsedQuery.joinBuffer = stoi(bufferString);

    string binaryOperator = tokenizedQuery[9];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.joinBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }

    if (parsedQuery.joinBuffer < 3)
    {
        cout << "SEMANTIC ERROR: Buffer Size too small for the algorithm." << endl;
        return false;
    }
    return true;
}


void executeNestedJOIN()
{
    logger.log("executeNestedJOIN");

    Table table1 = *tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table table2 = *tableCatalogue.getTable(parsedQuery.joinSecondRelationName);
    Cursor cursor1 = table1.getCursor();

    vector <string> resultColumns(table1.columns);
    for (uint i = 0; i < table2.columns.size(); i++)
        resultColumns.push_back(table2.columns[i]);
    Table *resultTable = new Table(parsedQuery.joinResultRelationName);
    resultTable->columns = resultColumns;
    resultTable->columnCount = resultColumns.size();
    resultTable->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * resultTable->columnCount));


    int firstColumnIndex = table1.getColumnIndex(parsedQuery.joinFirstColumnName);
    int secondColumnIndex = table2.getColumnIndex(parsedQuery.joinSecondColumnName);

    int table1_blocks_per_iteration = parsedQuery.joinBuffer - 2;
    int no_of_iterations = ceil((float)table1.blockCount / (float)table1_blocks_per_iteration);

    vector <vector <int>> table1_rows;
    vector <vector <int>> table2_rows;
    vector <vector <int>> result_rows;
    vector <int> row;

    if (table2.blockCount == 1)
    {
        Cursor cursor2 = table2.getCursor();
        for (uint row_num = 0; row_num < table2.rowsPerBlockCount[0]; row_num++)
                table2_rows.push_back(cursor2.getNext());
    }

    for (uint iter = 0; iter < no_of_iterations; iter++)
    {
        table1_rows.clear();
        for (uint page_num = iter * table1_blocks_per_iteration; page_num < (iter + 1) * table1_blocks_per_iteration && page_num < table1.blockCount; page_num++)
            for (uint row_num = 0; row_num < table1.rowsPerBlockCount[page_num]; row_num++)
                table1_rows.push_back(cursor1.getNext());
        
        if (table2.blockCount == 1)
        {   
            for (uint row_num = 0; row_num < table2_rows.size(); row_num++)
            {
                row = table2_rows[row_num];
                for (uint i = 0; i < table1_rows.size(); i++)
                {
                    if (evaluateBinOp(table1_rows[i][firstColumnIndex], row[secondColumnIndex], parsedQuery.joinBinaryOperator))
                    {
                        vector <int> new_row(table1_rows[i]);
                        new_row.insert(new_row.end(), row.begin(), row.end());
                        result_rows.push_back(new_row);
                        resultTable->rowCount++;
                    }
                    if (result_rows.size() == resultTable->maxRowsPerBlock)
                    {
                        bufferManager.writePage(resultTable->tableName, resultTable->blockCount, result_rows, result_rows.size());
                        resultTable->rowsPerBlockCount.push_back(result_rows.size());
                        resultTable->blockCount++;
                        result_rows.clear();
                    }
                }
            }
        }
        else
        {
            Cursor cursor2 = table2.getCursor();
            while (true)
            {
                row = cursor2.getNext();
                if (row.empty())
                    break;
                
                for (uint i = 0; i < table1_rows.size(); i++)
                {
                    if (evaluateBinOp(table1_rows[i][firstColumnIndex], row[secondColumnIndex], parsedQuery.joinBinaryOperator))
                    {
                        vector <int> new_row(table1_rows[i]);
                        new_row.insert(new_row.end(), row.begin(), row.end());
                        result_rows.push_back(new_row);
                        resultTable->rowCount++;
                    }
                    if (result_rows.size() == resultTable->maxRowsPerBlock)
                    {
                        bufferManager.writePage(resultTable->tableName, resultTable->blockCount, result_rows, result_rows.size());
                        resultTable->rowsPerBlockCount.push_back(result_rows.size());
                        resultTable->blockCount++;
                        result_rows.clear();
                    }
                }
            }
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

void executeParthashJOIN()
{
    logger.log("executeParthashJOIN");
    cout << "executeParthashJOIN" << endl;
    return;
}


void executeJOIN()
{
    logger.log("executeJOIN");
    if (parsedQuery.joinAlgorithm == "NESTED")
        executeNestedJOIN();
    else if (parsedQuery.joinAlgorithm == "PARTHASH")
        executeParthashJOIN();
    return;
}