#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct MemoryNodeHeader {
        unsigned int blockSize;
        unsigned int dataSize;
        struct MemoryNodeHeader *parent;
        struct MemoryNodeHeader *leftChild;
        struct MemoryNodeHeader *rightChild;
        unsigned int isNotFree;
        // used as padding bytes for data allignment
        unsigned int paddingBytes;
        struct MemoryNodeHeader* nextRoot;
};

void allocation(unsigned int size, char c){
        void *mem_1 = malloc(size);
        struct MemoryNodeHeader* node_1 = (struct MemoryNodeHeader* ) mem_1 - 1;
        assert(node_1->dataSize >= size);
        assert(memset(mem_1, c, size-1) != NULL);
        memset(mem_1 + size-1, '\0', 1);
        printf("%s\n", mem_1);
}
void testScenarioMalloc(){
        // Testing Heap Consistency for consequetive mallocs
        allocation(16, 'a');
        allocation(16, 'b');
        allocation(16, 'c');
        allocation(2000, 'd');
        allocation(2000, 'e');
}

int main(int argc, char **argv)
{
        testScenarioMalloc();
        return 0;
}
