#include <unistd.h>
#include <stdio.h>
int count = 0;
int main(int argc, char const *argv[]) {
    while (1) {
        printf("%d\n", ++count);
        sleep(1);
        fflush(stdout);
    }
}
