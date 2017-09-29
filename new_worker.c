#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/epoll.h>
#include<sys/types.h>

void createNewWorker(
    char file[], int worker_index, int term, int fds[][2]) {

    //printf("Worker: %d , Term: %d\n", worker_index, term + 1);
    int pipefds[2];
    if(pipe(pipefds) == -1) {
        printf("%s\n", strerror(errno));
        exit(1);
    }
    fds[worker_index][0] = pipefds[0];
    fds[worker_index][1] = pipefds[1];

    // printf("Input file descriptor: %d\n", pipefds[0]);
    // printf("output file descriptor: %d\n", pipefds[1]);

    pid_t child_pid;
    child_pid = fork();
    if( child_pid == -1) {
        printf("%s\n", strerror(errno));
        exit(1);
    }
    if (child_pid == 0) {
        dup2(fds[worker_index][1], 1);
        char args_1[5] = "-x";
        char args_2[5] = "2";
        char args_3[5] = "-n";
        char args_4[100];
        sprintf(args_4, "%d", term);
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
float result = 0;

void waitForWorker(
    char file[], int fds[][2], int i, int num_workers, int num_terms, int* termsFound) {
    printf("term found val%d\n", *termsFound);
    fd_set rfds;

    while(1) {
        FD_ZERO(&rfds);
        for (int j=0; j<num_workers; j++) {
            if(fds[j][0] != -1) {
                printf("value set %d\n", fds[j][0]);
                FD_SET(fds[j][0], &rfds);
            }
        }
        int rds = select(
            getHighestNumberFd(fds, num_workers) + 1, &rfds, NULL, NULL, NULL);
        if (rds == -1) {
            printf("%s\n", strerror(errno));
            exit(1);
        }
        int j = 0;
        for (; j<num_workers; j++) {
            if (FD_ISSET(fds[j][0], &rfds) == 1) {
                char buffer[8];
                size_t size = 8;
                int c = 0;
                c = read(fds[j][0], buffer, size);
                printf("read %s\n\n\n", buffer);
                result+=atof(buffer);
                *termsFound = *termsFound + 1;
                printf("Number of terms found: %d\n",*termsFound);
                printf("Got result from the worke %d\n", fds[j][0]);
                printf("Here %f\n", result);
                close(fds[j][0]);

                if(i <= num_terms) {
                    createNewWorker(file, j, i, fds);
                    i++;
                } else {
                    fds[j][0] = -1;
                    break;
                }
            }
        }


        if(*termsFound == num_terms) {
            break;
        }


    }
}

int main(int argc, char const *argv[]) {
    int num_workers = 2;
    int num_terms = 11;
    char file[100] = "/home/shubhi/Desktop/Computer-Systems/worker";
    int i = 0;
    int termsFound = 0;

    if(num_workers > num_terms){
        num_workers = num_terms;
    }
    int fds[num_workers][2];
    for(int k=0;k<num_workers;k++) fds[k][0] = -1;
    while (i < num_workers) {
        createNewWorker(file, i, i, fds);
        i++;
    }
    waitForWorker(file, fds, i, num_workers, num_terms, &termsFound);

    printf("result %f\n", result);
}
