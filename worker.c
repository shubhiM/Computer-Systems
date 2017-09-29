#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char *argv[]) {
    //printf("here");
    if (argc != 5) {
        printf("%s\n", "Arguments required: -x 2 -n 3");
        exit(1);
    }
    char *x_s =  argv[2];
    int x = atoi(x_s);
    char *n_s = argv[4];
    int n = atoi(n_s);

    long double x_pow_n = 1;
    long double n_factorial = 1;
    long double result = 0.0;
    int a = n;
    if (n == 0) result = 1;

    else if(n == 1) result = x;

    else {

        while(n > 0)
        {
            x_pow_n = x_pow_n * (long double) x;
            n_factorial = n_factorial * (long double) n;
            n--;
        }
        result = x_pow_n/n_factorial;
    }
    if (isatty(1)) {
        printf("%d%s%d%s%Lf%s%Lf\n", x, "^", a , "/", n_factorial, " : ", result);
    }
    else {
         printf("%Lf\n",result);
    }
}
