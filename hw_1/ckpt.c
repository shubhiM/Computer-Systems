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
int flag = 0;

void signal_handler() {
        checkpoint();
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

void checkpoint() {
    FILE *ifp = NULL;
    FILE *ofp = NULL;
    ifp = fopen("/proc/self/maps", "r");
    ofp = fopen("myckpt", "wb+");
    if (ofp == NULL) {
        printf("Error opening output file: %s\n", strerror(errno));
        exit(1);
    }

    void * vsyscall_start_add;
    vsyscall_start_add = (void *) strtoll("ffffffffff600000", NULL, 16);
    ucontext_t context;
    int context_result = 0;
    context_result = getcontext(&context);
    if(context_result == -1) {
        printf("Error in getting context: %s\n", strerror(errno));
        exit(1);
    }

    if (flag == 1) {
        return;
    } else {
        flag++;
    }
    fwrite(&context, sizeof(ucontext_t), 1, ofp);
    if (ifp != NULL) {
        char line[1024];
        int c;
        int i = 0;
        while((c = getc(ifp)) != EOF) {
            if(c != '\n' && i < 1024) {
                line[i] = c;
                i++;
            } else {
                line[i++] = '\n';
                line[i] = '\0';
                struct MemoryRegion memregion = parseLine(line, i);
                // skipping regions that are not readable
                // skipping vsyscall
                if(memregion.isReadable && memregion.startAddr != vsyscall_start_add) {
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
        printf("%s\n", "checkpint taken");

    } else {
        printf("%s\n", "cant open the file");
        exit(1);
    }
}
