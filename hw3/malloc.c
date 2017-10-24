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
        int visited;
} *root;


void myprint(void* val){
        char buf[1024];
        snprintf(buf, 1024, "%p printed\n",(int *) val);
        write(STDOUT_FILENO, buf, strlen(buf) + 1);
}

struct MemoryNodeHeader* minNode = NULL;

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

void splitHelper(struct MemoryNodeHeader* node) {
        size_t newSize = node->blockSize/2;
        node->leftChild = node + 1;
        node->leftChild->parent = node;
        node->leftChild->blockSize = newSize;
        node->leftChild->isNotFree = 0;
        node->leftChild->visited = 0;
        node->rightChild = (void *) node + newSize;
        node->rightChild->parent = node;
        node->rightChild->blockSize = newSize;
        node->rightChild->isNotFree = 0;
        node->rightChild->visited = 0;
        node->isNotFree = 1;
        node->visited = 0;
}


int canBeSplit(struct MemoryNodeHeader* node, int requiredSize) {
        size_t newSize = (node->blockSize)/2;
        int leftNodeSize = newSize - 2*sizeof(struct MemoryNodeHeader);
        int rightNodeSize = newSize - sizeof(struct MemoryNodeHeader);
        return (!node->isNotFree && leftNodeSize > 0 && rightNodeSize > 0) && (
                       leftNodeSize >= requiredSize || rightNodeSize >= requiredSize);
}

void split(struct MemoryNodeHeader* node, size_t requiredSize) {
        // we do not have to split if we find the min node already
        if(!minNode && canBeSplit(node, requiredSize)) {
                splitHelper(node);
                split(node->leftChild, requiredSize);
                split(node->rightChild, requiredSize);

        } else {
                if(!minNode) {
                        minNode = node;
                        minNode->isNotFree = 1;
                }
        }
}

struct MemoryNodeHeader* freeNode = NULL;
// Function to get the free node from the Memory Tree
void getFreeNode(struct MemoryNodeHeader* node, size_t requiredSize) {
        if(!freeNode && node->leftChild && node->rightChild) {
                getFreeNode(node->leftChild, requiredSize);
                getFreeNode(node->rightChild, requiredSize);

        } else {
                if(!freeNode) {
                        if(!node->isNotFree) {
                                freeNode = node;
                        }
                }
        }
}

void createOrExtendMemoryTree(size_t requiredSize, int choice) {
        //struct MemoryNodeHeader* allocatedNode = NULL;
        if(choice == 1) {
                // create memory
                root = sbrk(0);
                // page size required to fulfill the memory request
                size_t pageSize = getPageSize(requiredSize);
                sbrk(pageSize);
                root->blockSize = pageSize;
                root->isNotFree = 0;
                root->visited = 0;
                root->parent = NULL;
                split(root, requiredSize);
        }
        else if(choice == 2) {
                // extend Memory
        }
        else {
                // exit with error saying invalid choice
        }
        //return NULL;
}

struct MemoryNodeHeader* dfs() {
        // returns the first free node by applying dfs from left to right
        return NULL;
}

void* allocateMemory(size_t requiredSize) {
        // set minNode to null
        // set freeNode to null
        minNode = NULL;
        freeNode = NULL;

        if(!root) {
                createOrExtendMemoryTree(requiredSize, 1);
                return minNode + 1;
        }
        else if(root->blockSize <= requiredSize) {
                //TODO: calculate exact data size to make it more
                // accurate
                // the root itself is not sufficient to hold the space
                // so we will have to extend the memory Tree
                createOrExtendMemoryTree(requiredSize, 2);
                return minNode + 1;
        } else {
                getFreeNode(root, requiredSize);
                if(freeNode) {
                        split(freeNode, requiredSize);
                        return minNode + 1;
                } else {
                        createOrExtendMemoryTree(requiredSize, 2);
                        return minNode + 1;
                }
        }
};

void * malloc(size_t requiredSize) {
        return allocateMemory(requiredSize);
}
