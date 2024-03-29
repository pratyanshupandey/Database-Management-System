#include"matrixPage.h"

/**
 * @brief The BufferManager is responsible for reading pages to the main memory.
 * Recall that large files are broken and stored as blocks in the hard disk. The
 * minimum amount of memory that can be read from the disk is a block whose size
 * is indicated by BLOCK_SIZE. within this system we simulate blocks by
 * splitting and storing the file as multiple files each of one BLOCK_SIZE,
 * although this isn't traditionally how it's done. You can alternatively just
 * random access to the point where a block begins within the same
 * file. In this system we assume that the the sizes of blocks and pages are the
 * same. 
 * 
 * <p>
 * The buffer can hold multiple pages quantified by BLOCK_COUNT. The
 * buffer manager follows the FIFO replacement policy i.e. the first block to be
 * read in is replaced by the new incoming block. This replacement policy should
 * be transparent to the executors i.e. the executor should not know if a block
 * was previously present in the buffer or was read in from the disk. 
 * </p>
 *
 */

enum Mode
{
    NORMAL,
    WRITEBACK
};

enum Policy
{
    FIFO,
    LRU
};

class MatrixBufferManager{

    deque<MatrixPage*> pages; 
    bool inPool(string pageName);
    MatrixPage* getFromPool(string pageName);
    MatrixPage* insertIntoPool(string matrixName, int pageIndex);

    public:
    Mode mode;
    Policy policy;
    MatrixBufferManager();
    MatrixPage* getPage(string matrixName, int pageIndex);
    void writePage(string pageName, vector<vector<int>> rows);      // unused unimplemented
    void deleteFile(string matrixName, int pageIndex);
    void deleteFile(string fileName);
    void writePage(string matrixName, int pageIndex, vector<ele_t> elements, uint N, uint maxElementsPerBlock);
    ele_t readWriteElement(string matrixName, int pageIndex, uint offset, ele_t value);
    void reset();
    ~MatrixBufferManager();
};