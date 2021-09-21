#include "global.h"

/**
 * @brief Construct a new Page object. Never used as part of the code
 *
 */
MatrixPage::MatrixPage()
{
    this->pageName = "";
    this->matrixName = "";
    this->pageIndex = -1;
    this->N = 0;
    this->max_elements = 0;
    this->elements.clear();
}

/**
 * @brief Construct a new MatrixPage:: MatrixPage object given the matrix name and page
 * index. When tables are loaded they are broken up into blocks of BLOCK_SIZE
 * and each block is stored in a different file named
 * "<matrixname>_Page<pageindex>". For example, If the Page being loaded is of
 * table "R" and the pageIndex is 2 then the file name is "R_Page2".
 * 
 * For purpose of loading an already filled page and reading from it.
 *
 * @param matrixName 
 * @param pageIndex 
 */
MatrixPage::MatrixPage(string matrixName, int pageIndex)
{
    logger.log("MatrixPage::MatrixPage");
    this->matrixName = matrixName;
    this->pageIndex = pageIndex;
    this->pageName = "../data/temp/" + this->matrixName + "_Page" + to_string(pageIndex);
    Matrix matrix = *matrixCatalogue.getMatrix(matrixName);
    this->max_elements = matrix.maxElementsPerBlock;
    this->N = matrix.N;
    this->elements.clear();

    ifstream fin(pageName, ios::in | ios::binary);
    ele_t element;
    while (!fin.eof())
    {
        fin.read(reinterpret_cast<char*>(&element), sizeof(element));
        this->elements.push_back(element);
    }
    fin.close();
    
    // this->rowCount = table.rowsPerBlockCount[pageIndex];
    // int number;
    // for (uint rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    // {
    //     for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
    //     {
    //         fin >> number;
    //         this->rows[rowCounter][columnCounter] = number;
    //     }
    // }
    // fin.close();
}

/**
 * @brief Construct a new MatrixPage:: MatrixPage object given all the details.
 * 
 * For purpose of creating a page and writing it in disk during load.
 *
 * @param matrixName 
 * @param pageIndex 
 * @param elements
 * @param N
 * @param maxElementsPerBlock
 */

MatrixPage::MatrixPage(string matrixName, int pageIndex, vector<ele_t> elements, uint N, uint maxElementsPerBlock)
{
    logger.log("MatrixPage::MatrixPage");
    this->matrixName = matrixName;
    this->pageIndex = pageIndex;
    this->N = N;
    this->pageName = "../data/temp/"+this->matrixName + "_Page" + to_string(pageIndex);
    this->elements = elements;
    this->max_elements = maxElementsPerBlock;
}


/**
 * @brief Get row from page indexed by rowIndex
 * 
 * @param rowIndex 
 * @return vector<int> 
 */
ele_t MatrixPage::getElement(int elementIndex)
{
    logger.log("MatrixPage::getElement");
    if(elementIndex >= this->elements.size())
        return -1;
    return this->elements[elementIndex];
}

/**
 * @brief writes current page contents to file.
 * 
 */
void MatrixPage::writePage()
{
    logger.log("MatrixPage::writePage");
    ofstream fout(this->pageName, ios::trunc | ios::binary);
    for (uint i = 0; i < this->elements.size(); i++)
    {
        ele_t element = this->elements[i];
        fout.write(reinterpret_cast<char * >(&element), sizeof(element));
    }
    fout.close();
    // for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    // {
    //     for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    //     {
    //         if (columnCounter != 0)
    //             fout << " ";
    //         fout << this->rows[rowCounter][columnCounter];
    //     }
    //     fout << endl;
    // }
    // fout.close();
}
