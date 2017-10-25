#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
        size_t size = 12; //128
        void *mem = malloc(size);
        //printf("Successfully malloc'd %zu bytes at addr %p\n", size, mem);
        assert(mem != NULL);
        // free(mem);
        // printf("Successfully free'd %zu bytes from addr %p\n", size, mem);
        size_t size_2 = 16; //128
        void *mem_2 = malloc(size_2);
        //printf("Successfully malloc'd %zu bytes at addr %p\n", size_2, mem_2);

        size_t size_3 = 16; // 256
        void *mem_3 = malloc(size_3);

        size_t size_4 = 2000; // 256
        void *mem_4 = malloc(size_4);

        // new root
        size_t size_5 = 2000; // 256
        void *mem_5 = malloc(size_5);

        // previous tree
        size_t size_7 = 900; // 256
        void *mem_7 = malloc(size_7);
        // new tree
        size_t size_8 = 900; // 256
        void *mem_8 = malloc(size_8);

        return 0;
}
