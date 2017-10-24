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
        node->leftChild->paddingBytes = 0;
        node->rightChild = (void *) node + newSize;
        node->rightChild->parent = node;
        node->rightChild->blockSize = newSize;
        node->rightChild->isNotFree = 0;
        node->rightChild->paddingBytes = 0;
        node->isNotFree = 1;
        node->paddingBytes = 0;
}

int canGoDown(struct MemoryNodeHeader* node, int requiredSize) {
        size_t newSize = (node->blockSize)/2;
        int leftNodeSize = newSize - 2*sizeof(struct MemoryNodeHeader);
        int rightNodeSize = newSize - sizeof(struct MemoryNodeHeader);
        return (leftNodeSize > 0 && rightNodeSize > 0) &&
               (leftNodeSize >= requiredSize || rightNodeSize >= requiredSize);
}

int canBeSplit(struct MemoryNodeHeader* node, int requiredSize) {
        return (!node->isNotFree && canGoDown(node, requiredSize));
}

void split(struct MemoryNodeHeader* node, size_t requiredSize) {
        // we do not have to split if we find the min node already
        if(!minNode && canBeSplit(node, requiredSize)) {
                splitHelper(node);
                split(node->leftChild, requiredSize);
                split(node->rightChild, requiredSize);

        } else {
                if(!minNode) {
                        //TODO: check size before assigning the minNode
                        // it can be either left or right child
                        minNode = node;
                        minNode->isNotFree = 1;
                }
        }
}

struct MemoryNodeHeader* freeNode = NULL;
// Function to get the free node from the Memory Tree
// The free node would be the most suitable free node that
// can fulfill the request
void getFreeNode(struct MemoryNodeHeader* node, size_t requiredSize) {
        if(!freeNode && node->leftChild && node->rightChild &&
           canGoDown(node, requiredSize)) {
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

void * malloc(size_t requiredSize) {
        return allocateMemory(requiredSize);
}
