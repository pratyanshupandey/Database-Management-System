#include "global.h"

MatrixBufferManager::MatrixBufferManager()
{
    logger.log("MatrixBufferManager::MatrixBufferManager");
}

/**
 * @brief Function called to read a MatrixPage from the buffer manager. If the MatrixPage is
 * not present in the pool, the MatrixPage is read and then inserted into the pool.
 *
 * @param matrixName 
 * @param pageIndex 
 * @return MatrixPage 
 */
MatrixPage MatrixBufferManager::getPage(string matrixName, int pageIndex)
{
    logger.log("MatrixBufferManager::getPage");
    string pageName = "../data/temp/"+matrixName + "_Page" + to_string(pageIndex);
    if (this->inPool(pageName))
        return this->getFromPool(pageName);
    else
        return this->insertIntoPool(matrixName, pageIndex);
}

/**
 * @brief Checks to see if a MatrixPage exists in the pool
 *
 * @param pageName 
 * @return true 
 * @return false 
 */
bool MatrixBufferManager::inPool(string pageName)
{
    logger.log("MatrixBufferManager::inPool");
    for (auto matrixPage : this->pages)
    {
        if (pageName == matrixPage.pageName)
            return true;
    }
    return false;
}

/**
 * @brief If the MatrixPage is present in the pool, then this function returns the
 * page. Note that this function will fail if the MatrixPage is not present in the
 * pool.
 *
 * @param pageName 
 * @return MatrixPage 
 */
MatrixPage MatrixBufferManager::getFromPool(string pageName)
{
    logger.log("MatrixBufferManager::getFromPool");
    for (auto matrixPage : this->pages)
        if (pageName == matrixPage.pageName)
            return matrixPage;
}

/**
 * @brief Inserts MatrixPage indicated by matrixName and pageIndex into pool. If the
 * pool is full, the pool ejects the oldest inserted MatrixPage from the pool and adds
 * the current MatrixPage at the end. It naturally follows a queue data structure. 
 *
 * @param matrixName 
 * @param pageIndex 
 * @return MatrixPage 
 */
MatrixPage MatrixBufferManager::insertIntoPool(string matrixName, int pageIndex)
{
    logger.log("MatrixBufferManager::insertIntoPool");
    MatrixPage page(matrixName, pageIndex);
    if (this->pages.size() >= BLOCK_COUNT)
        pages.pop_front();
    pages.push_back(page);
    return page;
}

/**
 * @brief The buffer manager is also responsible for writing pages. This is
 * called when new matrixs are created using assignment statements.
 *
 * @param matrixName 
 * @param pageIndex 
 * @param rows 
 * @param rowCount 
 */
void MatrixBufferManager::writePage(string matrixName, int pageIndex, vector<ele_t> elements, uint N, uint maxElementsPerBlock)
{
    logger.log("MatrixBufferManager::writePage");
    MatrixPage page(matrixName, pageIndex, elements, N, maxElementsPerBlock);
    page.writePage();
}

/**
 * @brief Deletes file names fileName
 *
 * @param fileName 
 */
void MatrixBufferManager::deleteFile(string fileName)
{
    
    if (remove(fileName.c_str()))
        logger.log("MatrixBufferManager::deleteFile: Err");
        else logger.log("MatrixBufferManager::deleteFile: Success");
}

/**
 * @brief Overloaded function that calls deleteFile(fileName) by constructing
 * the fileName from the matrixName and pageIndex.
 *
 * @param matrixName 
 * @param pageIndex 
 */
void MatrixBufferManager::deleteFile(string matrixName, int pageIndex)
{
    logger.log("MatrixBufferManager::deleteFile");
    string fileName = "../data/temp/"+matrixName + "_Page" + to_string(pageIndex);
    this->deleteFile(fileName);
}