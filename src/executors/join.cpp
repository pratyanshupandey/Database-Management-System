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

void NestedJOIN(Table table1, int firstIndex, Table table2, int secondIndex, Table* resultTable, vector <vector <int>> &result_rows, bool reverse)
{
    if (table1.blockCount == 0 || table2.blockCount == 0)
        return;

    int table1_blocks_per_iteration = parsedQuery.joinBuffer - 2;
    int no_of_iterations = ceil((float)table1.blockCount / (float)table1_blocks_per_iteration);
    Cursor cursor1 = table1.getCursor();

    vector <vector <int>> table1_rows;
    vector <vector <int>> table2_rows;
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
                    if (evaluateBinOp(table1_rows[i][firstIndex], row[secondIndex], parsedQuery.joinBinaryOperator))
                    {
                        vector <int> new_row(table1_rows[i]);
                        if (reverse)
                            new_row.insert(new_row.begin(), row.begin(), row.end());
                        else
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
                    if (evaluateBinOp(table1_rows[i][firstIndex], row[secondIndex], parsedQuery.joinBinaryOperator))
                    {
                        vector <int> new_row(table1_rows[i]);
                        if (reverse)
                            new_row.insert(new_row.begin(), row.begin(), row.end());
                        else
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

}

void executeNestedJOIN()
{
    logger.log("executeNestedJOIN");

    Table table1 = *tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table table2 = *tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    vector <string> resultColumns(table1.columns);
    for (uint i = 0; i < table2.columns.size(); i++)
        resultColumns.push_back(table2.columns[i]);
    Table *resultTable = new Table(parsedQuery.joinResultRelationName);
    resultTable->columns = resultColumns;
    resultTable->columnCount = resultColumns.size();
    resultTable->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * resultTable->columnCount));


    int firstColumnIndex = table1.getColumnIndex(parsedQuery.joinFirstColumnName);
    int secondColumnIndex = table2.getColumnIndex(parsedQuery.joinSecondColumnName);

    vector <vector <int>> result_rows;
    if(table1.blockCount <= table2.blockCount)
        NestedJOIN(table1, firstColumnIndex, table2, secondColumnIndex, resultTable, result_rows, false);
    else
        NestedJOIN(table2, secondColumnIndex, table1, firstColumnIndex, resultTable, result_rows, true);

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

    Table table1 = *tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table table2 = *tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    vector <string> resultColumns(table1.columns);
    for (uint i = 0; i < table2.columns.size(); i++)
        resultColumns.push_back(table2.columns[i]);
    Table *resultTable = new Table(parsedQuery.joinResultRelationName);
    resultTable->sourceFileName = "";
    resultTable->columns = resultColumns;
    resultTable->columnCount = resultColumns.size();
    resultTable->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * resultTable->columnCount));


    int firstColumnIndex = table1.getColumnIndex(parsedQuery.joinFirstColumnName);
    int secondColumnIndex = table2.getColumnIndex(parsedQuery.joinSecondColumnName);
    int partitions_count = parsedQuery.joinBuffer - 1;

    vector <int> row;

    /* 
    #########################
     Partition Phase
    #########################
     */
    // Table 1
    vector <Table *> table1_parts;
    for (uint i = 0; i < partitions_count; i++)
    {
        Table* new_bucket = new Table(table1.tableName + "_" + to_string(i));
        new_bucket->columns = table1.columns;
        new_bucket->columnCount = table1.columnCount;
        new_bucket->maxRowsPerBlock = table1.maxRowsPerBlock;
        table1_parts.push_back(new_bucket);
    }
    vector<vector<int>> partition_blocks[partitions_count];
    Cursor cursor1 = table1.getCursor();
    while(true)
    {
        row = cursor1.getNext();
        if (row.empty())
            break;
        int hash = row[firstColumnIndex] % partitions_count;
        partition_blocks[hash].push_back(row);
        table1_parts[hash]->rowCount++;
        if (partition_blocks[hash].size() == table1.maxRowsPerBlock)
        {
            bufferManager.writePage(table1_parts[hash]->tableName, table1_parts[hash]->blockCount, partition_blocks[hash], partition_blocks[hash].size());
            table1_parts[hash]->rowsPerBlockCount.push_back(partition_blocks[hash].size());
            table1_parts[hash]->blockCount++;
            partition_blocks[hash].clear();
        }
    }
    for (uint i = 0; i < partitions_count; i++)
    {
        if (!partition_blocks[i].empty())
            {
                bufferManager.writePage(table1_parts[i]->tableName, table1_parts[i]->blockCount, partition_blocks[i], partition_blocks[i].size());
                table1_parts[i]->rowsPerBlockCount.push_back(partition_blocks[i].size());
                table1_parts[i]->blockCount++;
                partition_blocks[i].clear();
            }
        tableCatalogue.insertTable(table1_parts[i]);
    }
    

    // Table 2

    vector <Table *> table2_parts;
    for (uint i = 0; i < partitions_count; i++)
    {
        Table* new_bucket = new Table(table2.tableName + "_" + to_string(i));
        new_bucket->columns = table2.columns;
        new_bucket->columnCount = table2.columnCount;
        new_bucket->maxRowsPerBlock = table2.maxRowsPerBlock;
        table2_parts.push_back(new_bucket);
    }
    Cursor cursor2 = table2.getCursor();
    while(true)
    {
        row = cursor2.getNext();
        if (row.empty())
            break;
        int hash = row[secondColumnIndex] % partitions_count;
        partition_blocks[hash].push_back(row);
        table2_parts[hash]->rowCount++;
        if (partition_blocks[hash].size() == table2.maxRowsPerBlock)
        {
            bufferManager.writePage(table2_parts[hash]->tableName, table2_parts[hash]->blockCount, partition_blocks[hash], partition_blocks[hash].size());
            table2_parts[hash]->rowsPerBlockCount.push_back(partition_blocks[hash].size());
            table2_parts[hash]->blockCount++;
            partition_blocks[hash].clear();
        }
    }
    for (uint i = 0; i < partitions_count; i++)
    {
        if (!partition_blocks[i].empty())
            {
                bufferManager.writePage(table2_parts[i]->tableName, table2_parts[i]->blockCount, partition_blocks[i], partition_blocks[i].size());
                table2_parts[i]->rowsPerBlockCount.push_back(partition_blocks[i].size());
                table2_parts[i]->blockCount++;
                partition_blocks[i].clear();
            }
        tableCatalogue.insertTable(table2_parts[i]);
    }
    
    /* 
    #########################
     Joining Phase
    #########################
     */
    
    vector <vector<int>> result_rows;
    for (uint part_num = 0; part_num < partitions_count; part_num++)
    {
        if(table1_parts[part_num]->blockCount <= table2_parts[part_num]->blockCount)
            NestedJOIN(*table1_parts[part_num], firstColumnIndex, *table2_parts[part_num], secondColumnIndex, resultTable, result_rows, false);
        else
            NestedJOIN(*table2_parts[part_num], secondColumnIndex, *table1_parts[part_num], firstColumnIndex, resultTable, result_rows, true);
        
        tableCatalogue.deleteTable(table1_parts[part_num]->tableName);
        tableCatalogue.deleteTable(table2_parts[part_num]->tableName);
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


void executeJOIN()
{
    logger.log("executeJOIN");
    if (parsedQuery.joinAlgorithm == "NESTED")
        executeNestedJOIN();
    else if (parsedQuery.joinAlgorithm == "PARTHASH")
        executeParthashJOIN();
    return;
}