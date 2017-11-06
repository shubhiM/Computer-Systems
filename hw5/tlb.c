#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>
#define PAGESIZE sysconf(_SC_PAGESIZE)
int a[1048576];

void initialize(int a[], int size){
        for(int i=0; i<size; i++) {
                a[i] = 0;
        }
}

double getTimeToAccessIthElement(int index){
        struct timespec start, end;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        a[index]+=1;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
        unsigned long start_time = 1000000000*start.tv_sec + start.tv_nsec;
        unsigned long end_time = 1000000000*end.tv_sec + end.tv_nsec;
        printf("Access time for %d element of array %lf\n",
               index, (double)(end_time - start_time));
        return (double) (end_time - start_time);
}

void specCacheLineSize(){
        for(int j=2; j<=10000; j=j*2) {
                getTimeToAccessIthElement(j);
                if(j == 1024 || j == 2048 || j == 4096 || j==8192) {
                        for(int k=1; k<=5; k++) {
                                getTimeToAccessIthElement(j+k);
                        }
                }
        }
}


void specCacheSize(int trials, int pages){
        struct timespec start, end;
        int jump = PAGESIZE/sizeof(int);
        double total = 0;
        double denom = 0.0;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        for (int t=0; t<trials; t++) {
                // Access each page once
                for(int s=0; s<pages*jump; s+=jump) {
                        // 0 to 1023 elements of the array belong
                        // to the same page
                        a[s]+=1;
                        denom += 1.0;
                }

        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
        unsigned long start_time = 1000000000*start.tv_sec + start.tv_nsec;
        unsigned long end_time = 1000000000*end.tv_sec + end.tv_nsec;
        unsigned long diff = end_time - start_time;
        total+=diff;
        printf("Average access time(nsec)/page %lf for reading %d pages over %d trials\n",
               total/(denom), pages, trials);
}


void specAssociativity(){

        // Cache full
        for(int i=0; i<8192*2; i++) {
                a[i]+=1;
        }

        for(int i=0; i<=8192*16; i=i+8192) {
                getTimeToAccessIthElement(i);
                printf("******************************************\n");
                for(int j=0; j<=i; j=j+8192) {
                        getTimeToAccessIthElement(j);
                }
                printf("******************************************\n");
        }
}

void runCacheSizeSpec(){
        initialize(a, 1048576);
        for(int i=2; i<=1024; ) {
                specCacheSize(100000000, i);
                i = i*2;
        }
}
int main(int argc, char const *argv[]) {
        cpu_set_t my_set;
        CPU_ZERO(&my_set);
        CPU_SET(0, &my_set);
        if (sched_setaffinity(getpid(), sizeof(my_set), &my_set) == -1) {
                printf("CPU affinity could not be set");
                exit(1);
        }
        runCacheSizeSpec();
        specCacheLineSize();
        //specAssociativity();

}
