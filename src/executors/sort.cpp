#include"global.h"
/**
 * @brief File contains method to process SORT commands.
 * 
 * syntax:
 * R <- SORT relation_name BY column_name IN sorting_order
 * 
 * sorting_order = ASC | DESC 
 * SYNTAX: <new_table_name> <- SORT <table_name> BY <column_name> IN ASC | DESC
 * <new_table_name> <- SORT <table_name> BY <column_name> IN ASC | DESC BUFFER <buffer_size
 */
bool syntacticParseSORT(){
    logger.log("syntacticParseSORT");

    if((tokenizedQuery.size() != 8 && tokenizedQuery.size() != 10) || tokenizedQuery[4] != "BY" || tokenizedQuery[6] != "IN")
    {
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }
    if(tokenizedQuery.size() == 10 && tokenizedQuery[8] != "BUFFER")
    {
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }

    parsedQuery.queryType = SORT;
    parsedQuery.sortResultRelationName = tokenizedQuery[0];
    parsedQuery.sortRelationName = tokenizedQuery[3];
    parsedQuery.sortColumnName = tokenizedQuery[5];
    string sortingStrategy = tokenizedQuery[7];

    if(sortingStrategy == "ASC")
        parsedQuery.sortingStrategy = ASC;
    else if(sortingStrategy == "DESC")
        parsedQuery.sortingStrategy = DESC;
    else{
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }

    if(tokenizedQuery.size() == 10)
    {
        string bufferString = tokenizedQuery[9];
        for (uint i = 0; i < bufferString.size(); i++)
            if(!isdigit(bufferString[i]))
            {
                cout << "SYNTAX ERROR" << endl;
                return false;
            }

        parsedQuery.sortBuffer = stoi(bufferString);
    }

    return true;
}

bool semanticParseSORT(){
    logger.log("semanticParseSORT");

    if(tableCatalogue.isTable(parsedQuery.sortResultRelationName))
    {
        cout<<"SEMANTIC ERROR: Resultant relation already exists"<<endl;
        return false;
    }

    if(!tableCatalogue.isTable(parsedQuery.sortRelationName))
    {
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    if(!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnName, parsedQuery.sortRelationName))
    {
        cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
        return false;
    }

    if(parsedQuery.sortBuffer < 3)
    {
        cout<<"SEMANTIC ERROR: Buffer Size too small for the algorithm."<<endl;
        return false;
    }

    return true;
}

bool sort_key(const vector <int> &a, const vector <int> &b)
{
    if(parsedQuery.sortingStrategy == ASC)
        return a[parsedQuery.sortColumnIndex] < b[parsedQuery.sortColumnIndex];
    else
        return a[parsedQuery.sortColumnIndex] > b[parsedQuery.sortColumnIndex];
}

class Comparator
{
    public:
    bool operator()(const pair <vector <int>, int> &a, const pair <vector <int>, int> &b)
    {
        if(parsedQuery.sortingStrategy == ASC)
            return a.first[parsedQuery.sortColumnIndex] > b.first[parsedQuery.sortColumnIndex];
        else
            return a.first[parsedQuery.sortColumnIndex] < b.first[parsedQuery.sortColumnIndex];
    }
};

void write_back_to_page(string tableName, int pageIndex, vector<vector<int>> rows, int begin, int end, int columnCount)
{
    logger.log("Page::write BAck to Page");
    string pageName = "../data/temp/"+ tableName + "_Page" + to_string(pageIndex);
    ofstream fout(pageName, ios::trunc);
    for (int rowCounter = begin; rowCounter < end; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
        {
            if (columnCounter != 0)
                fout << " ";
            fout << rows[rowCounter][columnCounter];
        }
        fout << endl;
    }
    fout.close();
}

void executeSORT(){
    logger.log("executeSORT");

    Table table = *tableCatalogue.getTable(parsedQuery.sortRelationName);

    Table *resultTable = new Table(parsedQuery.sortResultRelationName);
    resultTable->sourceFileName = "";

    parsedQuery.sortColumnIndex = table.getColumnIndex(parsedQuery.sortColumnName);


    /* ########################
    Sorting Phase
    ###########################*/

    int no_of_runs = ceil((float)table.blockCount / (float)parsedQuery.sortBuffer);
    int run_size = parsedQuery.sortBuffer;
    int begin, end;
    
    vector < vector <int>> table_rows;
    Cursor cursor = table.getCursor();
    queue <Table *> table_runs;

    for (uint run = 0; run < no_of_runs; run++)
    {
        for (uint page_num = run * run_size; page_num < (run + 1) * run_size && page_num < table.blockCount; page_num++)
                for (uint row_num = 0; row_num < table.rowsPerBlockCount[page_num]; row_num++)
                    table_rows.push_back(cursor.getNext());
        
        // Sort
        sort(table_rows.begin(), table_rows.end(), sort_key);

        // Write back in a new bucket
        Table* new_bucket = new Table(table.tableName + "__" + to_string(run));
        new_bucket->columns = table.columns;
        new_bucket->columnCount = table.columnCount;
        new_bucket->maxRowsPerBlock = table.maxRowsPerBlock;
        new_bucket->rowCount = table_rows.size();

        int block_count = ceil((float)table_rows.size() / (float) table.maxRowsPerBlock);
        for (uint block = 0; block < block_count; block++)
        {
            begin = block * table.maxRowsPerBlock;
            end = block == block_count - 1 ? table_rows.size() : (block + 1) * table.maxRowsPerBlock;
            write_back_to_page(new_bucket->tableName, new_bucket->blockCount, table_rows, begin, end, new_bucket->columnCount);
            new_bucket->rowsPerBlockCount.push_back(end - begin);
            new_bucket->blockCount++;
        }
         
        table_runs.push(new_bucket);
        tableCatalogue.insertTable(new_bucket);
        table_rows.clear();
    }


    /* ########################
    Merging Phase
    ###########################*/

    uint max_runs_per_merge_iter = parsedQuery.sortBuffer - 1;
    uint merge_iter = 0;
    vector <int> row;
    vector <vector <int>> result_rows;

    while(true)
    {
        uint no_of_runs = table_runs.size();
        
        for (uint merge_start = 0; merge_start < no_of_runs; merge_start += max_runs_per_merge_iter)
        {
            uint merge_len = min(max_runs_per_merge_iter, no_of_runs - merge_start);
            vector <Cursor> cursors;
            vector <Table *> tables;
            vector <vector<int>> block_rows[merge_len];
            vector <int> pointers; 
            vector <int> block_num; 


            Table* new_bucket;
            if (no_of_runs > max_runs_per_merge_iter)
                new_bucket = new Table(table.tableName + "__" + to_string(merge_iter) + "__" + to_string(merge_start));
            else
                new_bucket = resultTable;
            new_bucket->columns = table.columns;
            new_bucket->columnCount = table.columnCount;
            new_bucket->maxRowsPerBlock = table.maxRowsPerBlock;
            

            priority_queue <pair < vector <int>, int>, vector<pair < vector <int>, int>>, Comparator> pq;

            for (uint i = 0; i < merge_len; i++)
            {
                Table * temp = table_runs.front();
                table_runs.pop();
                Cursor temp_cursor = temp->getCursor();
                tables.push_back(temp);

                for (uint row_num = 0; row_num < temp->rowsPerBlockCount[0]; row_num++)
                    block_rows[i].push_back(temp_cursor.getNext());

                pq.push({block_rows[i][0], i});
                pointers.push_back(1);
                block_num.push_back(0);
                cursors.push_back(temp_cursor);
            }

            while(!pq.empty())
            {
                auto ele = pq.top();
                pq.pop();
                result_rows.push_back(ele.first);
                if (result_rows.size() == new_bucket->maxRowsPerBlock)
                {
                    bufferManager.writePage(new_bucket->tableName, new_bucket->blockCount, result_rows, result_rows.size());
                    new_bucket->rowsPerBlockCount.push_back(result_rows.size());
                    new_bucket->blockCount++;
                    new_bucket->rowCount += result_rows.size();
                    result_rows.clear();
                }
                uint idx = ele.second;
                if(block_num[idx] >= tables[idx]->blockCount)
                    continue;
                else if(pointers[idx] < block_rows[idx].size())
                {
                    pq.push({block_rows[idx][pointers[idx]], idx});
                    pointers[idx] += 1;
                }
                else
                {
                    block_rows[idx].clear();
                    block_num[idx] += 1;
                    if(block_num[idx] >= tables[idx]->blockCount)
                    {
                        continue;
                    }
                    else
                    {
                        for (uint row_num = 0; row_num < tables[idx]->rowsPerBlockCount[block_num[idx]]; row_num++)
                            block_rows[idx].push_back(cursors[idx].getNext());
                        pq.push({block_rows[idx][0], idx});
                        pointers[idx] = 1;
                    }
                }
            }
            if (!result_rows.empty())
            {
                bufferManager.writePage(new_bucket->tableName, new_bucket->blockCount, result_rows, result_rows.size());
                new_bucket->rowsPerBlockCount.push_back(result_rows.size());
                new_bucket->blockCount++;
                new_bucket->rowCount += result_rows.size();
                result_rows.clear();
            }

            for (uint i = 0; i < merge_len; i++)
            {
                tableCatalogue.deleteTable(tables[i]->tableName);
                // delete tables[i];
            }

            table_runs.push(new_bucket);
            tableCatalogue.insertTable(new_bucket);
            
        }
        merge_iter++;
        cout << merge_iter << endl;
        if (no_of_runs <= max_runs_per_merge_iter)
            break;
    }


    return;
}