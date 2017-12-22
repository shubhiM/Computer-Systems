#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <unistd.h>
#define PAGESIZE sysconf(_SC_PAGESIZE)
//4MB blocksize
#define BLOCKSIZE PAGESIZE*1024
#define DATASIZE BLOCKSIZE - sizeof(struct addressList)

const int PAGE_FAULT_FACTOR =  2;
const int WINDOWSIZE = 10;
unsigned long totalMemory = 0;
const int keys = 20000;
int max = -1;
int maxFrequency = -1;

// Linked List of allocated addresses
struct addressList {
        unsigned long memTime;
        struct addressList* next;
}*head;


void insertHashMap(int index, int hashMap[]){
        hashMap[index] = hashMap[index]+1;
        if(hashMap[index] > maxFrequency) {
                maxFrequency = hashMap[index];
                max = index;
        }
}


void printData(struct addressList* addr){
        char* ptr = (char*) (addr + 1);
        printf("value at %s\n", ptr);
}

double averagePageFaultPerWindow(int arr[]){
        double total = 0;
        for(int i=0; i<WINDOWSIZE; i++) {
                total+=arr[i];
        }
        return total/(double) WINDOWSIZE;
}

unsigned long getMemTime(struct addressList* addr, size_t size){
        struct timeval start, end;
        gettimeofday(&start, NULL);
        memset(addr + 1, 'a', size);
        gettimeofday(&end, NULL);
        unsigned long start_time = 1000000*start.tv_sec + start.tv_usec;
        unsigned long end_time = 1000000*end.tv_sec + end.tv_usec;
        unsigned long diff = end_time - start_time;
        return diff;
}

int getPageFaults(){
        int counter = 0;
        struct addressList* top = head;
        while(top) {
                unsigned long prevTime = top->memTime;
                unsigned long newTime = getMemTime(top, DATASIZE);
                if(newTime > PAGE_FAULT_FACTOR * prevTime) {
                        counter++;
                }
                top = top->next;
        }
        return counter;
}

void initializeHashMap(int hashMap[]){
        for(int j=0; j<keys; j++) {
                hashMap[j] = 0;
        }
}
int main(int argc, char  *argv[]) {
        struct addressList* ptr = NULL;
        int pageFaultWindow[WINDOWSIZE];
        int hashMap[keys];
        initializeHashMap(hashMap);
        int i = 0;
        while(1) {
                struct addressList *addr = (struct addressList*)malloc(BLOCKSIZE);
                unsigned long memTime = getMemTime(addr, DATASIZE);
                addr->memTime = memTime;
                addr->next = NULL;
                totalMemory+=BLOCKSIZE;
                if(!ptr) {
                        head = addr;
                        ptr = head;
                        pageFaultWindow[i%WINDOWSIZE] = 0;
                        insertHashMap(0, hashMap);
                }
                else {
                        ptr->next=addr;
                        ptr = ptr->next;
                        int numOfPageFaults = getPageFaults();
                        insertHashMap(numOfPageFaults, hashMap);
                        pageFaultWindow[i%WINDOWSIZE] = numOfPageFaults;
                        printf("Iteration %d, Page Faults (current) %d," \
                               " Average page faults(Last 20 iterations) %lf," \
                               " RAM consumed %lu, most frequent number of page faults %d," \
                               " frequency %d\n",
                               i, numOfPageFaults,
                               averagePageFaultPerWindow(pageFaultWindow),
                               totalMemory, max, maxFrequency);
                        if(averagePageFaultPerWindow(pageFaultWindow) > \
                           (double)20*max && max > 1) {
                                // We break when the number of pagefaults is
                                // consistently 20 times the number of pagefaults
                                // in the last 20 iterations
                                break;
                        }

                }
                i++;
        }
        printf("Estimated RAM size %lu\n", totalMemory);
}
