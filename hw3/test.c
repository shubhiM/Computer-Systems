#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
        size_t size = 12;
        void *mem = malloc(size);
        //printf("Successfully malloc'd %zu bytes at addr %p\n", size, mem);
        assert(mem != NULL);
        // free(mem);
        // printf("Successfully free'd %zu bytes from addr %p\n", size, mem);
        size_t size_2 = 16;
        void *mem_2 = malloc(size_2);
        //printf("Successfully malloc'd %zu bytes at addr %p\n", size_2, mem_2);
        return 0;
}
