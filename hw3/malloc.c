#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define PAGESIZE sysconf(_SC_PAGESIZE)

// Each memory block in a page is
// represented using this header
struct MemBlockHeader {
        size_t size;
        int isFree;
        struct MemBlockHeader *left;
        struct MemBlockHeader *right;
} *root;


struct MemBlockHeader* minRoot = NULL;
void  traverse(
        struct MemBlockHeader* root, size_t size) {
        if(minRoot) ;
        else {
                if (root && root->isFree && size == root->size) {
                        minRoot = root;
                }
                else if(root && root->left && root->left->size < size) {
                        // current root is the minRoot
                        if (root->left->isFree) {
                                minRoot = root;
                        }
                        else if (root->right && root->right->isFree) {
                                minRoot = root;
                        }
                }

                else if(root && root->isFree && size < root->size) {
                        traverse(root->left, size);
                        traverse(root->right, size);
                }
        }
}

void myprint(void* val){
        char buf[1024];
        snprintf(buf, 1024, "%p printed\n",val);
        write(STDOUT_FILENO, buf, strlen(buf) + 1);
}


void *allocatedAddress = NULL;

int sizeExtendsPage(size_t size) {
        return (size >= PAGESIZE);
}
void *allocatedForPageSize(size_t size) {
        if (size == PAGESIZE) {
                allocatedAddress = sbrk(0);
        }
        else if(size > PAGESIZE) {
                allocatedAddress = sbrk(0);
                int numPages;
                if (size%PAGESIZE == 0) {
                        numPages = size/PAGESIZE;
                } else {
                        numPages = size/PAGESIZE + size%PAGESIZE;
                }
                sbrk(numPages*PAGESIZE);
        }
        return allocatedAddress;
}
int power(int a, int p) {
        int result = 1;
        for (int i=1; i<=p; i++) {
                result = result * a;
        }
        return result;
}

void splitIntoBuddies(struct MemBlockHeader *node, int pageSize) {
        struct MemBlockHeader *left = node + 1;
        size_t dataSize = pageSize - sizeof(struct MemBlockHeader);
        left->size = dataSize;
        left->isFree = 1;
        left->left = NULL;
        left->right = NULL;
        node->left = left;
        //void *addr = (void *)node + pageSize/sizeof(addr) + 1;
        void *addr = (void *)node + pageSize;
        struct MemBlockHeader *right = addr;
        right->size = dataSize;
        right->isFree = 1;
        right->left = NULL;
        right->right = NULL;
        node->right = right;
}


struct MemBlockHeader* largestBuddy(size_t size) {
        root = sbrk(0);
        sbrk(PAGESIZE);
        size_t dataSize = PAGESIZE - sizeof(struct MemBlockHeader);
        root->size = dataSize;
        root->isFree = 1;
        int initalSize = PAGESIZE;
        int powOftwo = 1;
        initalSize = PAGESIZE/power(2, powOftwo);
        struct MemBlockHeader *currentNode = root;
        while(size < initalSize) {
                if(size > initalSize/power(2, powOftwo+1)) {
                        break;
                }
                splitIntoBuddies(currentNode, initalSize);
                currentNode = currentNode->left;
                //myprint(currentNode);
                initalSize = PAGESIZE/power(2, ++powOftwo);
        }
        // Node is occupied
        currentNode->isFree = 0;
        // return the address of the data segment of the largestBuddy
        return currentNode + 1;
}

void *allocateMemory(size_t size) {
        if (root && root->isFree) {
                // root tree exists and there is space in the root tree
                myprint(1);
                minRoot = NULL;
                traverse(root, size);
                myprint(minRoot);
                return minRoot;
        } else {
                // root tree does not exists
                // create a new one
                //TODO: if root exists but its full then add this new tree
                // to the linked list of memory trees
                myprint(3);
                return largestBuddy(size);
        }
}
void *malloc(size_t size) {
        return allocateMemory(size);
}
