#include<stdio.h>
#include<stdlib.h>
// #include<include/isa.h>
// #include<readline/readline.h>
// #include<readline/history.h>
#include<string.h>
#include"sdb.h"


int main()
{
    int i = 0;
    int j = 0;
    int k = 0;
    printf("What's your name?");
    // init_monitor();

    for(i = 0;i<100;i++)
    {
        j = j+2;
        printf("The i value is %d, The j value is %d",i,j);
        printf("The k value is %d",k);
    }
    return 0;

}

