#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/mman.h>


struct MemoryRegion
{
    void *startAddr;
    void *endAddr;
    int isReadable;
    int isWritable;
    int isExecutable;
};

char ckpt_image[1000];

//OX530000-OX530100

void restoring_memory(char ckpt_image[]) {
    //printf("%s\n", "I am restoring memory");
    int ifp;
    ifp = open(ckpt_image, O_RDONLY);
    int c = 0;
    struct MemoryRegion buffer;
    ucontext_t context_1;
    c = read(ifp, &context_1, sizeof(ucontext_t));
    off_t offset = 0;
    while((c = read(ifp, &buffer, sizeof(struct MemoryRegion))) != 0) {
        //printf("%s\n", "I am in the loop");
        // Add struct to the offset
        int prot = PROT_READ;

        if(buffer.isWritable) {
            prot = prot | PROT_WRITE;
        }
        if (buffer.isExecutable) {
            prot = prot | PROT_EXEC;
        }
        offset = offset + sizeof(struct MemoryRegion);
        size_t mem_size = buffer.endAddr - buffer.startAddr;
        mmap(buffer.startAddr, mem_size, prot, MAP_FIXED,ifp, offset);
        offset = offset + mem_size;
    }
    //printf("%s\n", "I am ending the restore");
}

void unmap_current_stack_() {

    // FILE* ifp;
    // ifp = fopen("/proc/self/maps", "r");
    //
    // // char buffer[1024];
    // // size_t  size = 1024;
    // int c;
    // char buffer[1024];
    // char stackmap[1024];
    // int i = 0;
    // while((c = getc(ifp)) != 0) {
    //     if(c != '\n'&& i < 1024) {
    //         buffer[i++] = c;
    //     } else {
    //         buffer[i++] = '\n';
    //         buffer[i] = '\0';
    //         int j = i;
    //         while(--j > 0) {
    //             if
    //         }
    //     }
    // }

}
void *addr;
int main(int argc, char *argv[]) {



    //void *addr;

    // saving the name of the checkpointing image
    // in a global variable

    // change the stack pointer of current program
    // to a new location that is unlikely to be taken
    // by any other process

    size_t size = 135168;
    addr = mmap(
        (void *)0x5300000,
        size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if (addr == MAP_FAILED) {
        printf("%s\n", "Stack address not able to allocate" );
        printf("%s\n", strerror(errno));
    }

    //printf("%s\n", "printing new stack address");
    printf("%p\n", addr);
    //asm volatile("mov %0,%%rsp"::"g"(addr): "memory");
    asm volatile("mov %0,%%rsp"::"g"(addr +  size): "memory");
    //sleep(120);
    //printf("%s\n", "I am here");
    //printf("%s\n", argv[1]);
    restoring_memory(argv[1]);
}
