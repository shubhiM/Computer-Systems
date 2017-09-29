#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/epoll.h>
#include<sys/types.h>

void createNewWorker(
    char file[], int worker_index, int term, int fds[][2]) {

    printf("Worker: %d , Term: %d\n", worker_index, term + 1);
    int pipefds[2];
    if(pipe(pipefds) == -1) {
        printf("%s\n", strerror(errno));
        exit(1);
    }
    fds[worker_index][0] = pipefds[0];
    fds[worker_index][1] = pipefds[1];

    printf("Input file descriptor: %d\n", pipefds[0]);
    printf("output file descriptor: %d\n", pipefds[1]);

    pid_t child_pid;
    child_pid = fork();
    if( child_pid == -1) {
        printf("%s\n", strerror(errno));
        exit(1);
    }
    if (child_pid == 0) {
        //close(fds[worker_index][0]);
        dup2(fds[worker_index][1], 1);
        char args_1[5] = "-x";
        char args_2[5] = "2";
        char args_3[5] = "-n";
        char args_4[100];
        sprintf(args_4, "%d", term + 1);
        if(execl(
            file, file, args_1, args_2, args_3, args_4, NULL) == -1) {
            printf("%s\n", strerror(errno));
            exit(1);
        }
    }
}

int getHighestNumberFd(int fds[][2], int num_workers){
    int max = fds[0][0];
    for (int i=0;i<num_workers;i++) {
        if (fds[i][0] >= max) max = fds[i][0];
    }
    return max;
}

void waitForWorker(
    char file[], int fds[][2], int i, int num_workers, int num_terms) {
    fd_set rfds;
    while(1) {
        //printf("%s\n", "..1");
        FD_ZERO(&rfds);
        //printf("%s\n", "..2");
        for (int j=0; j<num_workers; j++) {
            FD_SET(fds[j][0], &rfds);
        }
        //printf("%s\n", "..3");
        int rds = select(
            getHighestNumberFd(fds, num_workers), &rfds, NULL, NULL, NULL);
        //printf("%s\n", "..4");
        if (rds == -1) {
            printf("%s\n", strerror(errno));
            exit(1);
        }
        int found = 0;
        for (int j=0; j<num_workers; j++) {
            if (FD_ISSET(fds[j][0], &rfds)) {
                char buffer[8];
                size_t size = 8;
                int c = 0;
                c = read(fds[j][0], buffer, size);
                printf("Got result from the worker\n");
                printf("%s\n", buffer);
                // spawn a new worker
                // file, workerindex , term-1, fds
                createNewWorker(file, j, i, fds);
                found = 1;
                break;
            }
          if (found) break;
        }
        if (found) break;
    }
}

int main(int argc, char const *argv[]) {
    // Declarations
    const int num_workers = 2;
    const int num_terms = 3;
    char file[100] = "/home/shubhi/Desktop/Computer-Systems/worker";
    int i = 0;
    int fds[num_workers][2];

    while (i < num_workers) {
        createNewWorker(file, i, i, fds);
        i++;
    }
    while (i < num_terms) {
        waitForWorker(file, fds, i, num_workers, num_terms);
        i++;
    }
}
