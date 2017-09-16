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

// Global contructor that gets called even before main

void checkpoint();

void signal_handler() {
    checkpoint();
    printf("%s\n", "checkpointing completed");
}
__attribute__((constructor))
void myconstructor() {
    signal(SIGUSR2, signal_handler);
}
// Data structures
struct MemoryRegion
{
    void *startAddr;
    void *endAddr;
    int isReadable;
    int isWritable;
    int isExecutable;
};
/******************************************************************************/

long totalSizeR = 0;
long totalsizeRW = 0;

void totalSizeOfReadOnly(struct MemoryRegion mem){

    if (mem.isReadable && !mem.isWritable && !mem.isExecutable) {
        totalSizeR = totalSizeR + (mem.endAddr - mem.startAddr);
    }
}

void totalSizeOfReadWrite(struct MemoryRegion mem){

    if (mem.isReadable && mem.isWritable) {
        totalsizeRW = totalsizeRW + (mem.endAddr - mem.startAddr);
    }
}

struct MemoryRegion parseLine(char line[], int size) {
    //printf("%s\n", line);
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
    totalSizeOfReadOnly(memregion);
    totalSizeOfReadWrite(memregion);
    return memregion;
}

void checkpoint() {
    FILE *ifp = NULL;
    FILE *ofp = NULL;
    ifp = fopen("/proc/self/maps", "r");
    ofp = fopen("myckpt", "wb+");
    if (ofp == NULL) {
        printf("Error opening output file: %s\n", strerror(errno));
        exit(1);
    }

    ucontext_t context;
    int context_result = 0;
    context_result = getcontext(&context);
    if(context_result == -1) {
        printf("Error in getting context: %s\n", strerror(errno));
        exit(1);
    }
    printf("%s\n", "Writing context info in to checkpointing image");
    fwrite(&context, sizeof(ucontext_t), 1, ofp);
    //printf("%s %d\n", "size of getcontext struct", sizeof(ucontext_t));
    if (ifp != NULL) {
        char line[1024];
        int c;
        int i = 0;
        printf("%s\n", "Writing memory maps to the checkpointing image");
        while((c = getc(ifp)) != EOF) {
            if(c != '\n' && i < 1024) {
                line[i] = c;
                i++;
            } else {
                line[i++] = '\n';
                line[i] = '\0';
                //printf("%s\n", line);
                struct MemoryRegion memregion = parseLine(line, i);
		        //printf("%ld %ld\n", memregion.startAddr, memregion.endAddr);

                if(memregion.isReadable) {
                    fwrite(&memregion, sizeof(struct MemoryRegion), 1 , ofp);
                    fwrite(
                        memregion.startAddr, 1,
                        (memregion.endAddr - memregion.startAddr), ofp);
                }
                i = 0;
            }
        }
        fclose(ifp);
     	fclose(ofp);

    } else {
        printf("%s\n", "cant open the file");
    }
    printf("%s %ld\n", "total readonly address", totalSizeR);
    printf("%s %ld\n", "total read write address", totalsizeRW);

    int ifp2;
    ifp2 = open("myckpt", O_RDONLY);
    if(ifp2 == -1) {
        printf("%s\n", "Check pointing image not found");
        exit(1);
    }
    else {
            printf("%s\n", "Reading from checkpointing image");
        int c = 0;
    	struct MemoryRegion buffer;
        ucontext_t context_1;

        // reading first the getcontext of the process
        c = read(ifp2, &context_1, sizeof(ucontext_t));
        printf("%s %d\n", "Reading get context, size", c);
        printf("%s\n", "reading the mem blocks now" );
        while((c = read(ifp2, &buffer, sizeof(struct MemoryRegion))) != 0) {
            //printf("%ld %ld\n", buffer.startAddr, buffer.endAddr);
            char contentBuffer[buffer.endAddr - buffer.startAddr];
            c = read(ifp2, &contentBuffer, sizeof(contentBuffer));
            //fwrite(contentBuffer, sizeof(contentBuffer), 1 , stdout);
        }
    }
    close(ifp2);
}

int main(int argc , char** argv) {
	printf("%s\n", "I am in main");
}
