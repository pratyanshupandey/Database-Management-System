#include"logger.h"

typedef uint ele_t;

/**
 * @brief The Page object is the main memory representation of a physical page
 * (equivalent to a block). The page class and the page.h header file are at the
 * bottom of the dependency tree when compiling files. 
 */

class MatrixPage{

    string matrixName;
    string pageIndex;
    uint N;
    uint max_elements;
    vector<ele_t> elements;

    public:

    string pageName = "";
    MatrixPage();
    MatrixPage(string matrixName, int pageIndex);
    MatrixPage(string matrixName, int pageIndex, vector<ele_t> elements, uint N, uint maxElementsPerBlock);
    ele_t getElement(int elementIndex);
    void writePage();
};