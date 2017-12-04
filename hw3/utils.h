#include<unistd.h>

#ifndef __UTILS_H__
#define __UTILS_H__

typedef struct MemoryNodeHeader {
        unsigned int blockSize;
        unsigned int dataSize;
        struct MemoryNodeHeader *parent;
        struct MemoryNodeHeader *leftChild;
        struct MemoryNodeHeader *rightChild;
        unsigned int isNotFree;
        // used as padding bytes for data allignment
        unsigned int paddingBytes;
        struct MemoryNodeHeader* nextRoot;
} memoryNodeHeader;

// Each thread has an arena from which it allocates the memory
// head points to the arena
extern __thread memoryNodeHeader *head;

// Each thread keeps a minNode which is the most suitable node
// to fulfill memory request
extern __thread memoryNodeHeader* minNode;

// Each thread keeps a freeNode which is the first most suitable
// free node in the thread area.
extern __thread memoryNodeHeader* freeNode;

extern size_t getPageSize(size_t requiredSize);
extern unsigned int getLeftNodeSize(memoryNodeHeader* node);
extern unsigned int getRightNodeSize(memoryNodeHeader* node);
extern int canGoDown(memoryNodeHeader* node, int requiredSize);
extern int canBeSplit(memoryNodeHeader* node, int requiredSize);
extern int getNodeSize(memoryNodeHeader* node);
extern void splitHelper(memoryNodeHeader* node);
extern void split(memoryNodeHeader* node, size_t requiredSize);
extern void getFreeNode(memoryNodeHeader* node, size_t requiredSize);
extern void getFreeNode(memoryNodeHeader* node, size_t requiredSize);
extern void createMemoryTree(size_t requiredSize);
extern void* allocateMemory(size_t requiredSize);
extern int  isLeaf(memoryNodeHeader* node);
extern int  canBeMerged(memoryNodeHeader* node);
extern void freeHelper(memoryNodeHeader* node);
extern void freeWrapper(void *ptr);

#endif
