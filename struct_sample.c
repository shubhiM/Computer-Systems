#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


// Data structures
struct MemoryRegion
{
    void *startAddr;
    void *endAddr;
    int isReadable;
    int isWritable;
    int isExecutable;
};

char result[100];



/******************************************************************************/
// Main function
int main(int argc, char  *argv[]) {
    FILE *ifp;
    FILE *ofp;

    printf("I am here");
    if (ifp != NULL) {
        ofp = fopen("myckpt", "ab");
	    if (ofp == NULL) {
            printf("Error opening output file: %s\n", strerror(errno));
            exit(1);
        }
        char line[1024];
        size_t len = 1024;
        ssize_t read;
        read = getline(&line, &len, ifp);
        printf("%s\n", line);
        while(read != -1) {
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
            while(i <= read) {
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


            //TODO: check if the conversion from the hex to long int
            // is desirable
            // printf(
            //     "%ld %ld %d %d %d\n",
            //     memregion->startAddr,
            //     memregion->endAddr,
            //     memregion->isReadable,
            //     memregion->isWritable,
            //     memregion->isExecutable);

            //printf("%d\n", (memregion->endAddr - memregion->startAddr));
            //printf("%s\n", "*******************************" );
            fwrite(&memregion, sizeof(struct MemoryRegion), 1 , ofp);
            //free(memregion);
            //fwrite(memregion->startAddr, (memregion->endAddr - memregion->startAddr), 1 , ofp);
       	     char nextline[1024];
             size_t nextlen = 1024;
	     read = getline(&nextline, &nextlen, ifp);
	     printf("%s\n", nextline);

        fclose(ifp);
     	fclose(ofp);
     }
    } else {
        printf("%s\n", "cant open the file");
    }

}
