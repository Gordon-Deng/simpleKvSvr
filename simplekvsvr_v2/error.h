#ifndef ERROR_H
#define ERROR_H

#include<stdio.h>

#define ERROR(...) do {printf("ERROR:FILE:%s,LINE:%d  ",__FILE__,__LINE__);printf("%s\n",##__VA_ARGS__);} while(0)

#endif
