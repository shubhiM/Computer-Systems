#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>

#define PAGESIZE sysconf(_SC_PAGESIZE)

// Array is complely in memory
void experiment1(int trials, int pages){
        struct timespec start, end;
        int arraySize = (PAGESIZE/sizeof(int)) * pages;
        printf("size of array %d\n", arraySize);
        int a[arraySize];

        //Initialize array so that it gets into main
        //Memory
        for(int i=0; i<arraySize; i++) {
                a[i] = 0;
        }

        int jump = PAGESIZE/sizeof(int);
        double total = 0;
        double denom = 0.0;
        for (int t=0; t<trials; t++) {
                // Access each page once
                for(int s=0; s<pages*jump; s+=jump) {
                        // 0 to 1023 elements of the array belong
                        // to the same page
                        clock_gettime(CLOCK_REALTIME, &start);
                        a[s]+=1;
                        clock_gettime(CLOCK_REALTIME, &end);
                        unsigned long start_time = 1000000000*start.tv_sec + start.tv_nsec;
                        unsigned long end_time = 1000000000*end.tv_sec + end.tv_nsec;
                        unsigned long diff = end_time - start_time;
                        total+= diff;
                        denom += 1.0;
                        //printf("Trial %d, Time (ms) %lu, Stride: %d\n", t, diff, s);
                }
        }
        printf("Average access time(nsec)/page %lf for reading %d pages over %d trials\n",
               total/(denom), pages, trials);
}
int main(int argc, char const *argv[]) {
        /* code */

        cpu_set_t my_set;
        CPU_ZERO(&my_set);
        CPU_SET(0, &my_set);
        if (sched_setaffinity(getpid(), sizeof(my_set), &my_set) == -1) {
                printf("CPU affinity could not be set");
                exit(1);
        }

        for(int i=1; i<=25; ) {
                experiment1(10000000, i);
                i = i + 2;
        }
}
