#include "global.h"

/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Matrix:: Matrix object used in the case where the matrix
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param matrixName 
 */
Matrix::Matrix(string matrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
    this->maxElementsPerBlock = BLOCK_SIZE * 1024 / sizeof(ele_t);
    this->N = this->extractN();
    this->isSparse = this->checkSparse();
}

/**
 * @brief Function extracts N - the size of the n*n square matrix from the first
 *  line of the .csv data file. 
 *
 * @return true if N successfully extracted 
 * @return false otherwise
 */
uint Matrix::extractN()
{
    logger.log("Matrix::extractN");
    // unordered_set<string> columnNames;
    // string word;
    // stringstream s(firstLine);
    // while (getline(s, word, ','))
    // {
    //     word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
    //     if (columnNames.count(word))
    //         return false;
    //     columnNames.insert(word);
    //     this->columns.emplace_back(word);
    // }
    // this->columnCount = this->columns.size();
    // this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount));
    return true;
}


/**
 * @brief The checkSparse function is used when the LOAD command is encountered. It
 * reads data from the source file and decides what kind of matrix it is.
 *
 * @return true if the matrix is a sparse matrix 
 * @return false if matrix is a dense matrix
 */
bool Matrix::checkSparse()
{
    logger.log("Matrix::checkSparse");
    return false;
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file and splits it into blocks.
 *
 * @return true if the matrix has been successfully loaded 
 * @return false if an error occurred 
 */
bool Matrix::load()
{
    logger.log("Matrix::load");
    if (this->isSparse)
        return loadSparse();
    return loadDense();
}

/**
 * @brief The loadSparse function is used to load sparse matrices.
 *
 * @return true if the matrix has been successfully loaded 
 * @return false if an error occurred 
 */
bool Matrix::loadSparse()
{
    logger.log("Matrix::loadSparse");
    // fstream fin(this->sourceFileName, ios::in);
    // string line;
    // if (getline(fin, line))
    // {
    //     fin.close();
    //     if (this->extractColumnNames(line))
    //         if (this->blockify())
    //             return true;
    // }
    // fin.close();
    return false;
}

/**
 * @brief The loadDense function is used to load dense matrices.
 *
 * @return true if the matrix has been successfully loaded 
 * @return false if an error occurred 
 */
bool Matrix::loadDense()
{
    logger.log("Matrix::loadDense");
    fstream fin(this->sourceFileName, ios::in);
    
    string words, word;
    ele_t element;
    vector <ele_t> elements;

    if(!fin.is_open())
        return false;

    while (!fin.eof())
    {
        while (getline(fin, word, ','))
        {
            stringstream s(word);
            while (getline(s, word))
            {
                element = stoi(word);
                elements.push_back(element);
                if(elements.size() >= this->maxElementsPerBlock)
                {    // write in memeory
                    matrixBufferManager.writePage(this->matrixName, this->blockCount, elements, this->N, this->maxElementsPerBlock);
                    elements.clear();
                    this->blockCount++;
                }
            }
            
        }
        
    }
    fin.close();
    // string line;
    // if (getline(fin, line))
    // {
    //     fin.close();
    //     if (this->extractColumnNames(line))
    //         if (this->blockify())
    //             return true;
    // }
    // fin.close();
    return false;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    // ifstream fin(this->sourceFileName, ios::in);
    // string line, word;
    // vector<int> row(this->columnCount, 0);
    // vector<vector<int>> rowsInPage(this->maxRowsPerBlock, row);
    // int pageCounter = 0;
    // unordered_set<int> dummy;
    // dummy.clear();
    // this->distinctValuesInColumns.assign(this->columnCount, dummy);
    // this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    // getline(fin, line);
    // while (getline(fin, line))
    // {
    //     stringstream s(line);
    //     for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    //     {
    //         if (!getline(s, word, ','))
    //             return false;
    //         row[columnCounter] = stoi(word);
    //         rowsInPage[pageCounter][columnCounter] = row[columnCounter];
    //     }
    //     pageCounter++;
    //     this->updateStatistics(row);
    //     if (pageCounter == this->maxRowsPerBlock)
    //     {
    //         bufferManager.writePage(this->matrixName, this->blockCount, rowsInPage, pageCounter);
    //         this->blockCount++;
    //         this->rowsPerBlockCount.emplace_back(pageCounter);
    //         pageCounter = 0;
    //     }
    // }
    // if (pageCounter)
    // {
    //     bufferManager.writePage(this->matrixName, this->blockCount, rowsInPage, pageCounter);
    //     this->blockCount++;
    //     this->rowsPerBlockCount.emplace_back(pageCounter);
    //     pageCounter = 0;
    // }

    // if (this->rowCount == 0)
    //     return false;
    // this->distinctValuesInColumns.clear();
    return true;
}

/**
 * @brief Function prints the first few rows of the matrix. If the matrix contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Matrix::print()
{
    logger.log("Matrix::print");
    if (this->isSparse)
        this->printSparse();
    else
        this->printDense();
}

/**
 * @brief Function prints a sparse matrix
 */
void Matrix::printSparse()
{
    logger.log("Matrix::printSparse");
    // uint count = min((long long)PRINT_COUNT, this->rowCount);

    // //print headings
    // this->writeRow(this->columns, cout);

    // Cursor cursor(this->matrixName, 0);
    // vector<int> row;
    // for (int rowCounter = 0; rowCounter < count; rowCounter++)
    // {
    //     row = cursor.getNext();
    //     this->writeRow(row, cout);
    // }
    // printRowCount(this->rowCount);
}

/**
 * @brief Function prints a dense matrix
 */
void Matrix::printDense()
{
    logger.log("Matrix::printDense");    

    uint count = min(PRINT_COUNT, this->N);
    
    for (uint i = 0; i < count; i++)
    {
        uint ele_count = 0;
        while(ele_count < count)
        {
            int pageIndex = (i * this->N + ele_count) / this->maxElementsPerBlock;
            uint offset = (i * this->N + ele_count) % this->maxElementsPerBlock;
            MatrixPage page = matrixBufferManager.getPage(this->matrixName, pageIndex);
            while(offset < this->maxElementsPerBlock && ele_count < count)
                cout << page.getElement(offset) << " ";
        }

    }
    
    printRowCount(count);

    // //print headings
    // this->writeRow(this->columns, cout);

    // Cursor cursor(this->matrixName, 0);
    // vector<int> row;
    // for (int rowCounter = 0; rowCounter < count; rowCounter++)
    // {
    //     row = cursor.getNext();
    //     this->writeRow(row, cout);
    // }
    // printRowCount(this->rowCount);
}


/**
 * @brief This function transposes an already loaded matrix in place overwriting the
 * already present temp blocks
 * 
 * @return true if transpose succeeds 
 * @return false otherwise
 */
bool Matrix::transpose()
{
    logger.log("Matrix::transpose");
    if (this->isSparse) 
        return this->transposeSparse();
    return this->transposeDense();
}

/**
 * @brief This function transposes a sparse matrix
 * 
 * @return true if transpose succeeds 
 * @return false otherwise
 */
bool Matrix::transposeSparse()
{
    logger.log("Matrix::transposeSparse");
    return true;
}


/**
 * @brief This function transposes a dense matrix
 * 
 * @return true if transpose succeeds 
 * @return false otherwise
 */
bool Matrix::transposeDense()
{
    logger.log("Matrix::transposeDense");
}

/**
 * @brief This function returns one row of the matrix using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor 
 * @return vector<int> 
 */
void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNext");

        if (cursor->pageIndex < this->blockCount - 1)
        {
            cursor->nextPage(cursor->pageIndex+1);
        }
}



/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Matrix::makePermanent()
{
    logger.log("Matrix::makePermanent");
    if (this->isSparse)
        this->makePermanentSparse();
    else
        this->makePermanentDense();
}


/**
 * @brief Function for exporting a sparse matrix
 *
 */
void Matrix::makePermanentSparse()
{
    logger.log("Matrix::makePermanentSparse");
    // if(!this->isPermanent())
    //     bufferManager.deleteFile(this->sourceFileName);
    // string newSourceFile = "../data/" + this->matrixName + ".csv";
    // ofstream fout(newSourceFile, ios::out);

    // //print headings
    // this->writeRow(this->columns, fout);

    // Cursor cursor(this->matrixName, 0);
    // vector<int> row;
    // for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    // {
    //     row = cursor.getNext();
    //     this->writeRow(row, fout);
    // }
    // fout.close();
}

/**
 * @brief Function for exporting a dense matrix
 *
 */
void Matrix::makePermanentDense()
{
    logger.log("Matrix::makePermanentDense");

    if(!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->matrixName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    int row_ele_count = 0;
    for (int pageIndex = 0; pageIndex < this->blockCount; pageIndex++)
    {
        MatrixPage page = matrixBufferManager.getPage(this->matrixName, pageIndex);
        for (int elementIndex = 0; elementIndex < this->maxElementsPerBlock; elementIndex++)
        {
            fout << page.getElement(elementIndex);
            row_ele_count++;
            if(row_ele_count == this->N)
            {
                row_ele_count = 0;
                fout << "\n";
            }
            else
                fout << ",";

        }
        
    }
    fout.close();
    


    // //print headings
    // this->writeRow(this->columns, fout);

    // Cursor cursor(this->matrixName, 0);
    // vector<int> row;
    // for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    // {
    //     row = cursor.getNext();
    //     this->writeRow(row, fout);
    // }
    // fout.close();
}

/**
 * @brief Function to check if matrix is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Matrix::isPermanent()
{
    logger.log("Matrix::isPermanent");
    if (this->sourceFileName == "../data/" + this->matrixName + ".csv")
    return true;
    return false;
}

/**
 * @brief The unload function removes the matrix from the database by deleting
 * all temporary files created as part of this matrix
 *
 */
void Matrix::unload(){
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
    // if (!isPermanent())
    //     bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this matrix
 * 
 * @return Cursor 
 */
Cursor Matrix::getCursor()
{
    logger.log("Matrix::getCursor");
    Cursor cursor(this->matrixName, 0);
    return cursor;
}
/**
 * @brief Function that returns the index of column indicated by columnName
 * 
 * @param columnName 
 * @return int 
 */
// int Matrix::getColumnIndex(string columnName)
// {
//     logger.log("Matrix::getColumnIndex");
//     for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
//     {
//         if (this->columns[columnCounter] == columnName)
//             return columnCounter;
//     }
// }
