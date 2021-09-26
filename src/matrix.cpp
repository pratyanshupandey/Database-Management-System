#include "global.h"
uint max_size = 25 * 1e4;

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
 * @return N if successfully extracted 
 * @return 0 otherwise
 */
uint Matrix::extractN()
{
    logger.log("Matrix::extractN");
    fstream fin(this->sourceFileName, ios::in);
    string firstLine;
    if(getline(fin, firstLine))
    {
        fin.close();
        string word;
        uint size = 0;
        stringstream s(firstLine);
        while (getline(s, word, ','))
            size++;
        logger.log(to_string(size));
        return size;
    }
    fin.close();
    return 0;
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
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    uint zeroCount = 0;
    while(getline(fin, line))
    {
        stringstream s(line);
        while(getline(s, word, ','))
        {
            word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
            if(word == "0")
                zeroCount++;
        }
    }
    if(zeroCount >= 0.6*(this->N)*(this->N))
        return true;
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
    if(this->N == 0)
    {
        cout << "Matrix is empty" << endl;
        return false;
    } 
    if(this->isSparse)
        return this->loadSparse();
    return this->loadDense();
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
    ifstream fin(this->sourceFileName, ios::in);

    if(!fin.is_open())
        return false;
    
    string line, word;
    vector<ele_t> elements;
    int i = 0;

    while(getline(fin, line))
    {
        stringstream s(line);
        int j = 0;

        while(getline(s, word, ','))
        {
            word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
            if(word != "0")
            {
                elements.push_back(stoi(word));
                elements.push_back(i * this->N + j);

                if(elements.size() + 2 > this->maxElementsPerBlock)
                {
                    matrixBufferManager.writePage(this->matrixName, this->blockCount, elements, this->N, this->maxElementsPerBlock);
                    this->blockCount++;
                    elements.clear();
                }
            }
            j++;
        }
        i++;
    }
    if(elements.size())
    {
        matrixBufferManager.writePage(this->matrixName, this->blockCount, elements, this->N, this->maxElementsPerBlock);
        this->blockCount++;
        elements.clear();
    }
    return true;
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
        while (getline(fin, words))
        {
            stringstream s(words);
            while (getline(s, word, ','))
            {
                element = stoi(word);
                elements.push_back(element);
                if(elements.size() >= this->maxElementsPerBlock)
                {   
                    matrixBufferManager.writePage(this->matrixName, this->blockCount, elements, this->N, this->maxElementsPerBlock);
                    elements.clear();
                    this->blockCount++;
                }
            }
        }
    }
    if(elements.size() > 0)
    {   
        matrixBufferManager.writePage(this->matrixName, this->blockCount, elements, this->N, this->maxElementsPerBlock);
        elements.clear();
        this->blockCount++;
    }
    fin.close();
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
    uint count = min(PRINT_COUNT, this->N);

    // if(this->isTranspose)
    // {
    vector<vector<ele_t>> temp(count, vector<ele_t>(count, 0));
    uint pageIndex = 0;
    while (pageIndex < this->blockCount)
    {
        MatrixPage* page = matrixBufferManager.getPage(this->matrixName, pageIndex);
        
        vector <ele_t> elements = page->getVector();
        for (uint i = 0; i < elements.size(); i+=2)
        {
            ele_t val = elements[i];
            uint index = elements[i+1];

            if(index >= 20 * this->N)
                break;
            
            uint k = index / this->N;
            uint l = index % this->N;

            if(k < count && l < count)
                temp[k][l] = val;
            
        }
        pageIndex++;
    }
    for(uint i = 0; i < count; i++)
    {
        for(uint j = 0; j < count; j++)
            cout << temp[i][j] << " ";
        cout << endl;
    }
    // }
    // else
    // {
    //     uint i = 0;
    //     uint j = 0;
    //     uint offset = 0;
    //     uint pageIndex = 0;

    //     MatrixPage* page = matrixBufferManager.getPage(this->matrixName, pageIndex);

    //     while(i < count)
    //     {
    //         uint element = page->getElement(offset);
    //         uint index = page->getElement(offset + 1);
    //         uint k = index / this->N;
    //         uint l = index % this->N;

    //         while(i < k && i < count)
    //         {
    //             while(j < count)
    //             {
    //                 cout << "0" << " ";
    //                 j++;
    //             }
                    
    //             cout << endl;
    //             j = 0;
    //             i++;
    //         }

    //         if(i < count)
    //         {
    //             while (j < min(l, count))
    //             {
    //                 cout << "0" << " ";
    //                 j++;
    //             }

    //             if(j == count)
    //             {
    //                 cout << endl;
    //                 j = 0;
    //                 i++;
    //             }    
    //         }

    //         if(i < count && l < count)
    //         {    
    //             cout << element << " ";
    //             j++;
    //             if(j == count)
    //             {
    //                 cout << endl;
    //                 i++;
    //                 j = 0;
    //             }
    //         }

    //         if(offset + 4 > this->maxElementsPerBlock)
    //         {
    //             pageIndex++;
    //             if(pageIndex < this->blockCount)
    //                 page = matrixBufferManager.getPage(this->matrixName, pageIndex);
    //             else
    //             {
    //                 while(i < count)
    //                 {
    //                     while (j < count)
    //                     {
    //                         cout << "0" << " ";
    //                         j++;
    //                     }
    //                     cout << endl;
    //                     j = 0;
    //                     i++;
    //                 }   
    //             }
    //             offset = 0;
    //         }
    //         else
    //             offset+=2;
    //     }
    // }
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
            MatrixPage* page = matrixBufferManager.getPage(this->matrixName, pageIndex);
            while(offset < this->maxElementsPerBlock && ele_count < count)
            {
                cout << page->getElement(offset) << " ";
                ++offset;
                ++ele_count;
            }
        }
        cout << endl;
    }
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
    this->isTranspose = !this->isTranspose;
    matrixBufferManager.mode = WRITEBACK;
    matrixBufferManager.reset();

    uint pageIndex = 0;

    while (pageIndex < this->blockCount)
    {
        MatrixPage* page = matrixBufferManager.getPage(this->matrixName, pageIndex);
        vector<pair<ele_t, ele_t>> temp;
        vector <ele_t> elements = page->getVector();
        for (uint i = 0; i < elements.size(); i+=2)
        {
            uint val = elements[i];
            uint index = elements[i + 1];
            uint newIndex = (index % this->N) * N + (index / this->N);
            temp.emplace_back(newIndex, val);
        }
        sort(temp.begin(), temp.end());
        for (uint i = 0; i < temp.size(); i++)
        {
            page->setElement(2 * i, temp[i].second);
            page->setElement(2 * i + 1, temp[i].first);
        }
        pageIndex++;
    }
    
    matrixBufferManager.reset();
    matrixBufferManager.mode = NORMAL;
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

    // matrixBufferManager.mode = WRITEBACK;
    
    // MatrixPage curPage;
    // int curIndex;
    // fstream file;
    // for (uint i = 0; i < this->N; i++)
    // {
    //     for (uint j = 0; j < this->N; j++)
    //     {
            
    //     }
    // }

    matrixBufferManager.mode = WRITEBACK;
    matrixBufferManager.reset();
    
    int curPageIndex = 0, newPageIndex, pageIndex2;
    uint offset1, offset2;
    ele_t element, value;
    MatrixPage* cur_page = matrixBufferManager.getPage(this->matrixName, curPageIndex);
    
    for (uint i = 0; i < this->N - 1; i++)
    {
        for (uint j = i + 1; j < this->N; j++)
        {
            newPageIndex = (i * this->N + j) / this->maxElementsPerBlock;
            offset1 = (i * this->N + j) % this->maxElementsPerBlock;
            if (newPageIndex != curPageIndex)
            {
                cur_page->writePage();
                cur_page = matrixBufferManager.getPage(this->matrixName, newPageIndex);
                curPageIndex = newPageIndex;
            }
            value = cur_page->getElement(offset1);

            pageIndex2 = (j * this->N + i) / this->maxElementsPerBlock;
            offset2 = (j * this->N + i) % this->maxElementsPerBlock;
            if(pageIndex2 == curPageIndex)
            {
                element = cur_page->getElement(offset2);
                cur_page->setElement(offset2, value);
            }
            else
                element = matrixBufferManager.readWriteElement(this->matrixName, pageIndex2, offset2, value);

            cur_page->setElement(offset1, element);
        }
    }
    matrixBufferManager.reset();
    matrixBufferManager.mode = NORMAL;
    return true;


    // matrixBufferManager.mode = WRITEBACK;
    // int pageIndex1, pageIndex2, offset1, offset2;
    // ele_t element1, element2;
    // MatrixPage* page1;
    // MatrixPage* page2;
    // for (uint i = 0; i < this->N - 1; i++)
    // {
    //     for (uint j = i + 1; j < this->N; j++)
    //     {
    //         pageIndex1 = (i * this->N + j) / this->maxElementsPerBlock;
    //         offset1 = (i * this->N + j) % this->maxElementsPerBlock;
    //         page1 = matrixBufferManager.getPage(this->matrixName, pageIndex1);
    //         element1 = page1->getElement(offset1);
 
    //         pageIndex2 = (j * this->N + i) / this->maxElementsPerBlock;
    //         offset2 = (j * this->N + i) % this->maxElementsPerBlock;
    //         page2 = matrixBufferManager.getPage(this->matrixName, pageIndex2);
    //         element2 = page2->getElement(offset2);
            
    //         page1 = matrixBufferManager.getPage(this->matrixName, pageIndex1);
    //         page1->setElement(offset1, element2);
    //         page2 = matrixBufferManager.getPage(this->matrixName, pageIndex2);
    //         page2->setElement(offset2, element1);
    //     }
        
    // }
    
    // matrixBufferManager.mode = NORMAL;
    // return true;
    
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
    
    if(this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    logger.log("Matrix::makePermanentSparse");
    string newSourceFile = "../data/" + this->matrixName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    if(this->isTranspose)
    {
        uint maxRows = max_size / this->N;
        uint premax = maxRows;
        uint iterations = this->N / maxRows;
        uint row = 0;
        vector<uint> offset(this->blockCount, 0);
        for(uint i = 0; i <= iterations; i++)
        {
            if(i == iterations)
                maxRows = this->N % maxRows;
            row += maxRows;
            vector<vector<ele_t>> temp(maxRows, vector<ele_t>(this->N, 0));
            uint pageIndex = 0;
            while (pageIndex < this->blockCount)
            {
                MatrixPage* page = matrixBufferManager.getPage(this->matrixName, pageIndex);
                vector <ele_t> elements = page->getVector();
                while ((offset[pageIndex] + 2 <= elements.size()))
                {
                    uint val = elements[offset[pageIndex]];
                    uint index = elements[offset[pageIndex] + 1];
                    
                    if(index >= (row * this->N))
                        break;
                    
                    uint k = (index / this->N) % premax;
                    uint l = index % this->N;

                    temp[k][l] = val;
                    
                    offset[pageIndex]+=2;
                }
                pageIndex++;
            }
            for(uint i = 0; i < maxRows; i++)
            {
                for(uint j = 0; j < this->N; j++)
                {
                    fout << temp[i][j];
                    if(j == this->N - 1)
                        fout << endl;
                    else
                        fout << ",";
                }
            }
        }
    }
    else
    {
        uint i = 0;
        uint j = 0;
        uint offset = 0;
        uint pageIndex = 0;

        MatrixPage* page = matrixBufferManager.getPage(this->matrixName, pageIndex);

        while(i < this->N)
        {
            uint element = page->getElement(offset);
            uint index = page->getElement(offset + 1);
            uint k = index / this->N;
            uint l = index % this->N;

            while(i < k && i < this->N)
            {
                while(j < this->N)
                {
                    fout << "0";
                    j++;
                    
                    if(j == this->N)
                        fout << endl;
                    else
                        fout << ",";
                }
                    
                j = 0;
                i++;
            }

            if(i < this->N)
            {
                while (j < l)
                {
                    fout << "0";
                    j++;

                    if(j == this->N)
                    {
                        fout << endl;
                        j = 0;
                        i++;
                    }
                    else
                        fout << ",";
                }    
            }

            if(i < this->N)
            {    
                fout << element;
                j++;
                if(j == this->N)
                {
                    fout << endl;
                    i++;
                    j = 0;
                }
                else
                    fout << ",";
            }

            if(offset + 4 > this->maxElementsPerBlock)
            {
                pageIndex++;
                if(pageIndex < this->blockCount)
                    page = matrixBufferManager.getPage(this->matrixName, pageIndex);
                else
                {
                    while(i < this->N)
                    {
                        while (j < this->N)
                        {
                            fout << "0";
                            j++;

                            if(j == this->N)
                                fout << endl;
                            else
                                fout << ",";
                        }
                        j = 0;
                        i++;
                    }   
                }
                offset = 0;
            }
            else
                offset+=2;
        }
    }
    fout.close();
}

/**
 * @brief Function for exporting a dense matrix
 *
 */
void Matrix::makePermanentDense()
{
    logger.log("Matrix::makePermanentDense");

    if(this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->matrixName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    int row_ele_count = 0;
    for (int pageIndex = 0; pageIndex < this->blockCount; pageIndex++)
    {
        MatrixPage* page = matrixBufferManager.getPage(this->matrixName, pageIndex);
        vector <ele_t> elements = page->getVector();
        for (uint i = 0; i < elements.size(); i++)
        {
            fout << elements[i];
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
