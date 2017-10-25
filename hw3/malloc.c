#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define PAGESIZE sysconf(_SC_PAGESIZE)

struct MemoryNodeHeader {
        size_t blockSize;
        struct MemoryNodeHeader *parent;
        struct MemoryNodeHeader *leftChild;
        struct MemoryNodeHeader *rightChild;
        int isNotFree;
        // used as padding bytes for data allignment
        int paddingBytes;
        struct MemoryNodeHeader* nextRoot;
} *head;


struct MemoryNodeHeader* minNode = NULL;
struct MemoryNodeHeader* freeNode = NULL;


/******************************************************************************/
// HELPER FUNCTIONS

size_t getPageSize(size_t requiredSize) {
        // Total size that will be required will also include space for header
        size_t requiredSpace = requiredSize + sizeof(struct MemoryNodeHeader);
        // some of the page size will get occupied by the header
        size_t pageSize = PAGESIZE;
        size_t dataSegSize = pageSize - sizeof(struct MemoryNodeHeader);
        while(dataSegSize < requiredSpace) {
                pageSize = pageSize * 2;
                dataSegSize = pageSize - sizeof(struct MemoryNodeHeader);
        }
        return pageSize;
}

int getLeftNodeSize(size_t nodeSize){
        return nodeSize - 2*sizeof(struct MemoryNodeHeader);
}

int getRightNodeSize(size_t nodeSize){
        return nodeSize - sizeof(struct MemoryNodeHeader);
}

int canGoDown(struct MemoryNodeHeader* node, int requiredSize) {
        size_t newSize = (node->blockSize)/2;
        int leftNodeSize = getLeftNodeSize(newSize);
        int rightNodeSize = getRightNodeSize(newSize);
        return (leftNodeSize > 0 && rightNodeSize > 0) &&
               (leftNodeSize >= requiredSize || rightNodeSize >= requiredSize);
}

int canBeSplit(struct MemoryNodeHeader* node, int requiredSize) {
        return (!node->isNotFree && canGoDown(node, requiredSize));
}

int getNodeSize(struct MemoryNodeHeader* node){
        if(node->parent->leftChild == node) {
                return getLeftNodeSize(node->blockSize);
        }else{
                return getRightNodeSize(node->blockSize);
        }
}

void splitHelper(struct MemoryNodeHeader* node) {
        size_t newSize = node->blockSize/2;
        node->leftChild = node + 1;
        node->leftChild->parent = node;
        node->leftChild->blockSize = newSize;
        node->leftChild->isNotFree = 0;
        node->leftChild->paddingBytes = 0;
        node->rightChild = (void *) node + newSize;
        node->rightChild->parent = node;
        node->rightChild->blockSize = newSize;
        node->rightChild->isNotFree = 0;
        node->rightChild->paddingBytes = 0;
        node->isNotFree = 1;
        node->paddingBytes = 0;
}

void split(struct MemoryNodeHeader* node, size_t requiredSize) {
        // we do not have to split if we find the min node already
        if(!minNode && canBeSplit(node, requiredSize)) {
                splitHelper(node);
                split(node->leftChild, requiredSize);
                split(node->rightChild, requiredSize);

        } else {
                if(getNodeSize(node) >= requiredSize && !minNode) {
                        minNode = node;
                        minNode->isNotFree = 1;

                }
        }
}

// Function to get the free node from the Memory Tree
// The free node would be the most suitable free node that
// can fulfill the request
void getFreeNode(struct MemoryNodeHeader* node, size_t requiredSize) {
        if(!freeNode && node->leftChild && node->rightChild &&
           canGoDown(node, requiredSize)) {
                getFreeNode(node->leftChild, requiredSize);
                getFreeNode(node->rightChild, requiredSize);

        } else {
                if((getNodeSize(node) >= requiredSize) &&
                   !node->isNotFree && !freeNode) {
                        freeNode = node;
                }
        }
}

void createMemoryTree(size_t requiredSize) {
        struct MemoryNodeHeader* root = sbrk(0);
        // page size required to fulfill the memory request
        size_t pageSize = getPageSize(requiredSize);

        sbrk(pageSize);

        root->blockSize = pageSize;
        root->isNotFree = 0;
        root->paddingBytes = 0;
        root->parent = NULL;
        root->nextRoot = NULL;
        // head is pointing at the root
        if(head) {
                head->nextRoot = root;
        }else{
                head = root;
        }
        split(root, requiredSize);
}

void* allocateMemory(size_t requiredSize) {
        minNode = NULL;
        freeNode = NULL;
        if(head) {
                struct MemoryNodeHeader* temp = head;
                while(temp) {
                        // check in the temp mem tree
                        getFreeNode(temp, requiredSize);
                        if(freeNode) {
                                split(freeNode, requiredSize);
                                break;
                        }
                        temp = temp->nextRoot;
                }
                if(!minNode) {
                        createMemoryTree(requiredSize);
                }
        }
        else{
                createMemoryTree(requiredSize);
        }
        return minNode + 1;
}


void* malloc(size_t requiredSize) {
        return allocateMemory(requiredSize);
}
/*****************************************************************************/
// Implementing free

int isLeaf(struct MemoryNodeHeader* node){
        return (!node->leftChild && !node->rightChild);
}

int canBeMerged(struct MemoryNodeHeader* node){
        return ((node->leftChild && !node->leftChild->isNotFree) &&
                (node->rightChild && !node->rightChild->isNotFree));
}
void freeHelper(struct MemoryNodeHeader* node){
        if(isLeaf(node) || canBeMerged(node)) {
                node->isNotFree = 0;
                if(node->leftChild) {
                        node->leftChild = NULL;
                }
                if(node->rightChild) {
                        node->rightChild = NULL;
                }
                freeHelper(node->parent);
        }
}

void free(void* ptr) {
        struct MemoryNodeHeader* node = (struct MemoryNodeHeader*) ptr - 1;
        freeHelper(node);
}


/****************************************************************************/
// Implementing calloc

void* calloc(size_t nmem, size_t size){
        size_t totalSize = nmem * size;
        void* ptr = malloc(totalSize);
        return memset(ptr, 0, totalSize);
}

/*****************************************************************************/
// Implementing Realloc

void* realloc(void* ptr, size_t size){
        if(!ptr) {
                return malloc(size);
        }
        else if(ptr && size == 0) {
                free(ptr);
                return NULL;
        }
        else {
                // calculates the size of the data that needs to be copied to
                // new location for the required size
                // copies data from old to new location
                // frees the old location
                struct MemoryNodeHeader* node = (struct MemoryNodeHeader*)ptr - 1;
                size_t dataSize = 0;
                if(node->parent->leftChild == node) {
                        // this is left child
                        dataSize = node->blockSize - 2*sizeof(struct MemoryNodeHeader);
                }else{
                        // this is right child
                        dataSize = node->blockSize - sizeof(struct MemoryNodeHeader);
                }
                void *newPtr = memcpy(allocateMemory(size), ptr, dataSize);
                free(ptr);
                return newPtr;
        }
}
