#include <sys/types.h>
#include <string.h>
#include "utils.h"

void* calloc(size_t nmem, size_t size){
        size_t totalSize = nmem * size;
        void* ptr = allocateMemory(totalSize);
        return memset(ptr, 0, totalSize);
}
