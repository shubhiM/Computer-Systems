#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/epoll.h>
#include<sys/types.h>


long double result = 0;

void createNewWorker(
    char file[], int worker_index, int term, int fds[][2]) {
    int pipefds[2];
    if(pipe(pipefds) == -1) {
        printf("%s\n", strerror(errno));
        exit(1);
    }
    fds[worker_index][0] = pipefds[0];
    fds[worker_index][1] = pipefds[1];
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

void waitForWorker(
    char file[], int fds[][2], int i, int num_workers, int num_terms, int* termsFound) {
    fd_set rfds;

    while(1) {
        FD_ZERO(&rfds);
        for (int j=0; j<num_workers; j++) {
            if(fds[j][0] != -1) {
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
                char *topstr;
                c = read(fds[j][0], buffer, size);
                result+=strtold(buffer, & topstr);
                *termsFound = *termsFound + 1;
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


int createNewWorkerForEpoll(char file[], int term) {
    int pipefds[2];
    if(pipe(pipefds) == -1) {
        printf("%s\n", strerror(errno));
        exit(1);
    }
    pid_t child_pid;
    child_pid = fork();
    if( child_pid == -1) {
        printf("1%s\n", strerror(errno));
        exit(1);
    }
    if (child_pid == 0) {
        dup2(pipefds[1], 1);
        char args_1[5] = "-x";
        char args_2[5] = "2";
        char args_3[5] = "-n";
        char args_4[100];
        sprintf(args_4, "%d", term);
        if(execl(
            file, file, args_1, args_2, args_3, args_4, NULL) == -1) {
            printf("2%s\n", strerror(errno));
            exit(1);
        }
    }
    // return the read fd for the pipe created
    return pipefds[0];
}

void waitForWorkerEpoll(
    int num_workers, int* epfd, struct epoll_event ev[], int num_terms, char file[], int i) {
    int termsRead = 0;
    while(1) {
        int r = epoll_wait(*epfd, ev, num_workers, 10000);
        if(r < 0) {
            //TODO
        }
        for (int j=0; j<r;j++){
            char buffer[8];
            if(read(ev[j].data.fd, buffer, 8) == -1) {
                printf("3%s\n", strerror(errno));
                exit(1);
            }
            else {
                float readData = atof(buffer);
                result += readData;
                termsRead++;
                // remove the read fd from the monitor list of fd from epoll
                // events
                int unmap_result = epoll_ctl(
                    *epfd, EPOLL_CTL_DEL, ev[j].data.fd, &ev[j]);
                if(unmap_result == -1) {
                    printf("5.%s\n", strerror(errno));
                    exit(1);
                }
                close(ev[j].data.fd);
                if(i < num_terms) {
                    // termsRead is still pending
                    int rdfd = createNewWorkerForEpoll(file, i);
                    i++;
                    struct epoll_event e;
                    e.events = EPOLLIN;
                    e.data.fd = rdfd;
                    // add this rdfd to epoll_ctl
                    int er = epoll_ctl(*epfd, EPOLL_CTL_ADD, rdfd, &e);
                    if(er == -1) {
                        printf("6.%s\n", strerror(errno));
                        exit(1);
                    }
                }
                if(termsRead == num_terms) {
                    break;
                }
            }
        }
        if(termsRead  == num_terms) {
            break;
        }
    }

}

int main(int argc, char const *argv[]) {
    int num_workers = 4;
    //TODO: add extra 1 in numterm to the num terms passed by the user
    int num_terms = 13;
    char file[100] = "./worker";
    int i = 0;
    int termsFound = 0;
    int mechanism = 1;
    if(num_workers > num_terms){
        num_workers = num_terms;
    }
    int fds[num_workers][2];
    struct epoll_event r_ev[num_workers];
    int epfd = epoll_create(num_workers);
    if(mechanism == 0) {
        // select
        for(int k=0;k<num_workers;k++) fds[k][0] = -1;
        while (i < num_workers) {
            createNewWorker(file, i, i, fds);
            i++;
        }
        waitForWorker(file, fds, i, num_workers, num_terms, &termsFound);
    } else {
        // epoll
        while (i < num_workers) {
            int readFd = createNewWorkerForEpoll(file, i);
            struct epoll_event e;
            e.events = EPOLLIN;
            e.data.fd = readFd;
            int r = epoll_ctl(epfd, EPOLL_CTL_ADD, readFd, &e);
            if(r == -1) {
                printf("%s\n", strerror(errno));
                exit(1);
            }
            i++;
        }
        waitForWorkerEpoll(
            num_workers, &epfd, r_ev, num_terms, file, i);
    }
    printf("result %Lf\n", result);
}
