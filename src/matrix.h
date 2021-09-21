#include "cursor.h"

typedef uint ele_t;

/**
 * @brief The Table class holds all information related to a loaded table. It
 * also implements methods that interact with the parsers, executors, cursors
 * and the buffer manager. There are typically 2 ways a table object gets
 * created through the course of the workflow - the first is by using the LOAD
 * command and the second is to use assignment statements (SELECT, PROJECT,
 * JOIN, SORT, CROSS and DISTINCT). 
 *
 */
class Matrix
{

public:
    string sourceFileName = "";
    string matrixName = "";
    uint N;
    uint blockCount = 0;
    uint maxElementsPerBlock = 0;
    bool isSparse = false;

    Matrix();
    Matrix(string matrixName);

    bool checkSparse();
    uint extractN();
    bool blockify();

    bool load();
    bool loadSparse();
    bool loadDense();

    void print();
    void printSparse();
    void printDense();

    void makePermanent();
    void makePermanentSparse();
    void makePermanentDense();

    bool isPermanent();

    bool transpose();
    bool transposeSparse();
    bool transposeDense();


    void getNextPage(Cursor *cursor);
    Cursor getCursor();
    int getColumnIndex(string columnName);
    void unload();

    /**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row 
 */
template <typename T>
void writeRow(vector<T> row, ostream &fout)
{
    logger.log("Table::printRow");
    for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
    {
        if (columnCounter != 0)
            fout << ", ";
        fout << row[columnCounter];
    }
    fout << endl;
}

/**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row 
 */
template <typename T>
void writeRow(vector<T> row)
{
    logger.log("Table::printRow");
    ofstream fout(this->sourceFileName, ios::app);
    this->writeRow(row, fout);
    fout.close();
}
};