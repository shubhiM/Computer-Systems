#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "utils.h"
#define PAGESIZE sysconf(_SC_PAGESIZE)


// Mutex lock to manage the process heap data structure
pthread_mutex_t mutex;

// Each thread has an arena from which it allocates the memory
// head points to the arena
__thread memoryNodeHeader *head = NULL;

// Each thread keeps a minNode which is the most suitable node
// to fulfill memory request
__thread memoryNodeHeader* minNode = NULL;

// Each thread keeps a freeNode which is the first most suitable
// free node in the thread arean
__thread memoryNodeHeader* freeNode = NULL;



/******************************************************************************/
// HELPER FUNCTIONS

size_t getPageSize(size_t requiredSize) {
        // Total size that will be required will also include space for header
        size_t requiredSpace = requiredSize + sizeof(memoryNodeHeader);
        // some of the page size will get occupied by the header
        size_t pageSize = PAGESIZE;
        size_t dataSegSize = pageSize - sizeof(memoryNodeHeader);
        while(dataSegSize < requiredSpace) {
                pageSize = pageSize * 2;
                dataSegSize = pageSize - sizeof(memoryNodeHeader);
        }
        return pageSize;
}

unsigned int getLeftNodeSize(memoryNodeHeader* node){
        // Left node data size is equal to datasize - size of split
        // block - header for the node
        unsigned int splitSize = (node->blockSize)/2;
        unsigned int dataSize = node->dataSize - splitSize - sizeof(
                memoryNodeHeader);
        return dataSize;
}

unsigned int getRightNodeSize(memoryNodeHeader* node){
        unsigned int splitSize = (node->blockSize)/2;
        return splitSize - sizeof(memoryNodeHeader);
}

int canGoDown(memoryNodeHeader* node, int requiredSize) {
        int leftNodeSize = getLeftNodeSize(node);
        int rightNodeSize = getRightNodeSize(node);
        return (leftNodeSize > 0 && rightNodeSize > 0) &&
               (leftNodeSize >= requiredSize || rightNodeSize >= requiredSize);
}

int canBeSplit(memoryNodeHeader* node, int requiredSize) {
        return (!node->isNotFree && canGoDown(node, requiredSize));
}

int getNodeSize(memoryNodeHeader* node){
        if(node->parent->leftChild == node) {
                return getLeftNodeSize(node->parent);
        }else{
                return getRightNodeSize(node->parent);
        }
}

void splitHelper(memoryNodeHeader* node) {
        size_t newSize = node->blockSize/2;
        node->leftChild = node + 1;
        node->leftChild->parent = node;
        node->leftChild->blockSize = newSize;
        node->leftChild->dataSize = getLeftNodeSize(node);
        node->leftChild->isNotFree = 0;
        node->leftChild->paddingBytes = 0;
        node->rightChild = (void *)node + newSize;
        node->rightChild->parent = node;
        node->rightChild->blockSize = newSize;
        node->rightChild->dataSize = getRightNodeSize(node);
        node->rightChild->isNotFree = 0;
        node->rightChild->paddingBytes = 0;
        node->isNotFree = 1;
        node->paddingBytes = 0;
}

void split(memoryNodeHeader* node, size_t requiredSize) {
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
void getFreeNode(memoryNodeHeader* node, size_t requiredSize) {
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

        // TODO: Only one thread should do sbrk at a time
        // This is done to avoid concurrency issues.
        pthread_mutex_lock(&mutex);
        memoryNodeHeader* root = (memoryNodeHeader*) sbrk(0);
        // page size required to fulfill the memory request
        size_t pageSize = getPageSize(requiredSize);
        sbrk(pageSize);
        pthread_mutex_unlock(&mutex);

        root->blockSize = pageSize;
        root->dataSize = pageSize - sizeof(memoryNodeHeader);
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
                memoryNodeHeader* temp = head;
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

/************************************************************************************/
int isLeaf(memoryNodeHeader* node){
        return (!node->leftChild && !node->rightChild);
}

int canBeMerged(memoryNodeHeader* node){
        return ((node->leftChild && !node->leftChild->isNotFree) &&
                (node->rightChild && !node->rightChild->isNotFree));
}
void freeHelper(memoryNodeHeader* node){
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

void freeWrapper(void *ptr){
        memoryNodeHeader* node = (memoryNodeHeader*) ptr - 1;
        freeHelper(node);
}

void* malloc(size_t requiredSize) {
        return allocateMemory(requiredSize);
}
