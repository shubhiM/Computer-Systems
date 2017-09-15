#include <unistd.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {
    /* code */
    while (1) {
        printf("%c\n", '.');
        sleep(1);
        fflush(stdout);
    }
}
