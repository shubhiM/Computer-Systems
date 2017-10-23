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

struct MemoryRegion parseLine(char line[], int size) {
    char startadd[256];
    char endadd[256];
    int isReadable = 0;
    int isWritable = 0;
    int isExecutable = 0;
    int hc = 0;
    int i = 0;
    int start = 0;
    int end = 0;
    int sc =0;
    while(i <= size) {
        if(line[i] == '-') hc++;
        if(line[i] == ' ') sc++;
        if (line[i] != '-' && hc == 0) {
            startadd[start] = line[i];
            start++;
        }
        if(hc == 1 && line[i] == '-') {
            i++;
            continue;
        }
        if(sc == 1 && line[i] == ' ') {
            i++;
            continue;
        }
        if(hc == 1 && sc == 0 && hc!= '-') {
            endadd[end] = line[i];
            end++;
        }

        if(sc == 1 && sc < 2 && line[i] != ' ') {
            if (line[i] == 'r') isReadable = 1;
            if (line[i] == 'w') isWritable = 1;
            if (line[i] == 'x') isExecutable = 1;
        }
        i++;
    }
    startadd[start] = '\0';
    endadd[end] = '\0';
    struct MemoryRegion memregion;
    memregion.startAddr =(void *) strtoll(startadd, NULL, 16);
    memregion.endAddr =  (void *) strtoll(endadd, NULL, 16);
    memregion.isReadable = isReadable;
    memregion.isExecutable = isExecutable;
    memregion.isWritable = isWritable;
    return memregion;
}


char ckpt_image[1000];
ucontext_t context_1;

void restoring_memory(char ckpt_image[]) {
    int ifp;
    ifp = open(ckpt_image, O_RDONLY);
    if(ifp == -1) {
          printf("%s\n", "could not open checkpoint image");
 	      printf("%s\n", strerror(errno));
	      exit(1);
    }
    int c = 0;
    struct MemoryRegion buffer;
    c = read(ifp, &context_1, sizeof(ucontext_t));
    if (c == -1) {
        printf("%s\n","count not read context from checkpoint" );
        printf("%s\n", strerror(errno));
        exit(1);
    }
    while((c = read(ifp, &buffer, sizeof(struct MemoryRegion))) > 0) {
        size_t mem_size = buffer.endAddr - buffer.startAddr;
            if (mmap(
                buffer.startAddr,
                mem_size,
                PROT_READ|PROT_WRITE|PROT_EXEC,
                MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) == MAP_FAILED) {
                    printf("%s\n", "mmap failure");
                    printf("%s\n", strerror(errno));
    		        exit(1);
            }
            c = read(ifp, buffer.startAddr, mem_size);
            if (c == -1) {
                printf("%s\n", "Reading data from checkpint to mmap region failed");
                printf("%s\n",strerror(errno));
                exit(1);
            }
            int perm = 0;
            if(buffer.isReadable) perm |= PROT_READ;
            if (buffer.isWritable) perm |= PROT_WRITE;
            if (buffer.isExecutable) perm |= PROT_EXEC;
            int protect_result = mprotect(buffer.startAddr, mem_size, perm);
            if(protect_result == -1) {
                printf("%s\n","mprotect failed");
                printf("%s\n", strerror(errno));
                exit(1);
            }
    }
   close(ifp);
}



struct MemoryRegion get_stack_mem_map(char buffer[]) {
    FILE* ifp;
    ifp = fopen("/proc/self/maps", "r");
    int c;
    char stackmap[1024];
    char compareTo[10];
    strcpy(compareTo, "]kcats[");
    int i = 0;
    struct MemoryRegion mem;
    while((c = getc(ifp)) != EOF) {
        if(c != '\n'&& i < 1024) {
            buffer[i++] = c;
        } else {
            buffer[i++] = '\n';
            buffer[i] = '\0';
            int j = i - 2;
            int k = j - 7;
            int a = 0;

            while(j > k) {
                 stackmap[a] = buffer[j];
                 a++;
                 j--;
             }
            stackmap[a] = '\0';
             if (strcmp(stackmap, compareTo) == 0) {
                 mem = parseLine(buffer, i);
                 break;
             }
             i = 0;
        }
    }
    fclose(ifp);
    return mem;
}
void *addr;
size_t size = 135168;

void unmap_stack() {
    char buffer[1024];
    struct MemoryRegion mem = get_stack_mem_map(buffer);
    int unmap_result = munmap(mem.startAddr, (mem.endAddr - mem.startAddr));
    if (unmap_result == -1) {
    	 printf("%s\n", "Error while unmapping stack address");
    	 printf("%s\n", strerror(errno));
	     exit(1);
    }
    restoring_memory(ckpt_image);

}

int main(int argc, char *argv[]) {

    strcpy(ckpt_image, argv[1]);
    addr = mmap(
        (void *)0x5300000,
        size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED, -1, 0);

    if (addr == MAP_FAILED) {
        printf("%s\n", "Stack address not able to allocate" );
        printf("%s\n", strerror(errno));
    }

    asm volatile("mov %0,%%rsp"::"g"(addr +  size): "memory");
    unmap_stack();
    setcontext(&context_1);
}
