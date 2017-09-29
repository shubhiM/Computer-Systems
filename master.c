#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/epoll.h>
#include<sys/types.h>


int main(int argc, char const *argv[]) {
    int num_workers = 5;
    int num_terms = 11;
    char file[100] = "/home/shubhi/Desktop/Computer-Systems/worker";
    int i = 0;
    char args_4[100];
    int fds[num_terms];
    int numActiveWorkers = 0;
    while(i < num_terms) {
        int pipefd2[2];
        pid_t child_pid;
        child_pid = fork();
        if(pipe(pipefd2) == -1) {
            printf("%s\n", strerror(errno));
            exit(1);
        }
        if( child_pid == -1) {
            printf("%s\n", strerror(errno));
            exit(1);
        }

        if (child_pid == 0) {
                // child writes data on the pipe
            close(pipefd2[0]); // close the unused read file descriptor
            dup2(pipefd2[1], 1); // redirecting the stdout of the child_pid
                                     // to write end of the pipe
            char args_1[5] = "-x";
            char args_2[5] = "2";
            char args_3[5] = "-n";
            sprintf(args_4, "%d", i+1);
            if(execl(
                file, file, args_1, args_2, args_3, args_4, NULL) == -1) {
                printf("%s\n", strerror(errno));
                exit(1);
            }
            numActiveWorkers++;
        }
        else {
            fd_set rfds;
            fds[i] = pipefd2[0]; // add read file descriptor to the global
            // list of file descriptors
            close(pipefd2[1]); // close the unused write end of pipe
            if (numActiveWorkers == num_workers){
                // workers are all busy
                // wait for atleast one of them to get free
                while(1) {
                    FD_ZERO(&rfds);
                    // set all the fds
                    int numOfFds = 0;
                    for(int j=0; j<=i; j++) {
                        // file descriptors that are negative are
                        // the ones that are already read. Skip those
                        if(fds[j] != -1) {
                            FD_SET(fds[j], &rfds);
                            numOfFds++;
                        }
                    }
                    // select, to wait indefinitely for readiness of the
                    // workers
                    int rds = select(numOfFds+1, &rfds, NULL, NULL, NULL);
                    if (rds == -1) {
                        printf("%s\n", strerror(errno));
                        exit(1);
                    }
                    // check fds for readiness
                    for(int j=0; j<=i; j++) {
                        if(FD_ISSET(fds[j], &rfds) && fds[j] != -1) {
                            // the fd is set for reading
                            // display the result on the
                            // stdout of the master
                            char buffer[100];
                            size_t size = 100;
                            int c = 0;
                            while((c = read(fds[j], buffer, size)) > 0) {
                                write(1, buffer, size);
                            }
                            numActiveWorkers--;
                            // Mark this file descriptor as read
                            fds[j] = -1;
                        }
                    }
                    if (numActiveWorkers < num_workers) {
                        // there was minimum of one worker ready
                        // we can now break this loop
                        break;
                    }
                 }
                i++;
             }
          }
      }
}
