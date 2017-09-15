#include "stdio.h"



int main(int argc, char const *argv[]) {
    char str1[] = "shubhi";
    char str2[] = "is";
    char str3[] = "here";
    char result[100];
    int count = 0;
    while((result[count] = str1[count]) != '\0') count++;
    //printf("%d\n",count);
    int i = 0;
    while((result[count] = str2[i]) != '\0') {
        i++;
        count++;
    }
    //printf("%d\n",count);
    i = 0;
    while((result[count] = str3[i]) != '\0') {
        i++;
        count++;
    }
    result[count] = '\0';
    printf("%s\n", result);
    printf("%d\n",count);
}
