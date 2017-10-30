#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#define BLOCKSIZE 1000000
#define DATASIZE BLOCKSIZE - sizeof(struct addressList)

struct addressList {
        struct addressList* next;
}*head;

size_t totalMemory = 0;
unsigned long clocktime = 0;
const int NUM_OF_BLOCKS = 1024 * 1024;

void printData(){
        struct addressList* top = head;
        while(top) {
                char* ptr = (char*) (top + 1);
                printf("value at %s\n", ptr);
                top=top->next;
        }
}

int main(int argc, char  *argv[]) {
        struct addressList *temp = head;
        //struct timespec time1, time2;
        struct timeval start, end;

        for(int i=1; i<=NUM_OF_BLOCKS; i++) {
                struct addressList *newAddr = (struct addressList*) malloc(BLOCKSIZE);
                gettimeofday(&start, NULL);
                //clock_t start = clock();
                //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
                memset(newAddr + 1, 'a', DATASIZE);

                gettimeofday(&end, NULL);
                unsigned long start_time = 1000000*start.tv_sec + start.tv_usec;
                unsigned long end_time = 1000000*end.tv_sec + end.tv_usec;
                //clock_t end  = clock();
                //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
                //clock_t diff = (end - start);
                unsigned long diff = end_time -start_time;
                clocktime+=diff;
                if(!temp) {
                        head = newAddr;
                        temp = head;
                        temp->next = NULL;
                }
                else {
                        temp->next=newAddr;
                        temp = temp->next;
                }
                totalMemory+=BLOCKSIZE;
                double avg = (double) clocktime/i;
                printf(
                        "Time difference (micro secs) %lu, Memory (bytes) %lu, Avg time/process %lf\n",
                        diff, totalMemory, avg);
        }
        temp->next = NULL;
}
