#include<stdlib.h>
#include<stdio.h>
#include<string.h>

void main()
{
	int i=0;
	unsigned int value=0;
	unsigned char bitto[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
	unsigned char a[]={0xff,0x00,0x7e};
	unsigned char *p;
	p=a;
	int q=0;
	for(;i<8;i++)
	{
		if(*(p+i%2)&bitto[i])
			value=value|(1<<q);
		q++;
	}
	printf("%d\r\n",value);

}
