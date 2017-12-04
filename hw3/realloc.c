#include <string.h>
#include "utils.h"

void* realloc(void* ptr, size_t size){
        if(!ptr) {
                return allocateMemory(size);
        }
        else if(ptr && size == 0) {
                freeWrapper(ptr);
                return NULL;
        }
        else {
                // calculates the size of the data that needs to be copied to
                // new location for the required size
                // copies data from old to new location
                // frees the old location
                memoryNodeHeader* node = (memoryNodeHeader *)ptr - 1;
                void *newPtr = memcpy(allocateMemory(size), ptr, node->dataSize);
                freeWrapper(ptr);
                return newPtr;
        }
}
