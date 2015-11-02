#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//#define  __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) printf("File: "__FILE__",Line: %04d: "format"\n",__LINE__,##__VA_ARGS__)
#else
#define	DEBUG(format,...)
#endif
 void main()
{
	char str[]="hello world";
	DEBUG("AAA,HAHA:%s",str);
	return;
}
