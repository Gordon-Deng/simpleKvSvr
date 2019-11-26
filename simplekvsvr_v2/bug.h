#ifndef BUG_H
#define BUG_H

#include<stdio.h>

#define BUG() printf("FILE:%s,LINE:%d\n",__FILE__,__LINE__)

#endif
