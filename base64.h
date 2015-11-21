#include <stdio.h>
#include<stdlib.h>
#include<string.h>

static char base64_index[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

char* base64_encode_v1(const char * input,const size_t length,char* output)
{
	*output = '\0';
	if(input == NULL||length<1)
		return output;
	char*p = (char*)input;
	char*p_dst = (char*)output;
	char*p_end = (char*)input+length;
	int loop_count = 0;
	while(p_end-p >= 3)
	{
		*p_dst++=base64_index[(p[0] >> 2)];
		*p_dst++=base64_index[((p[0] << 4) & 0x30) | (p[1] >> 4)];
		*p_dst++=base64_index[((p[1] << 2) & 0x3c) | (p[2] >> 6)];
		*p_dst++=base64_index[p[2] & 0x3f];
		p += 3;
	}
	if (p_end-p > 0)
	{
		*p_dst++= base64_index[(p[0] >> 2)];
		if(p_end-p==2)
		{
			*p_dst++ = base64_index[((p[0] << 4) & 0x30) | (p[1] >>4)];
			*p_dst++ = base64_index[(p[1] << 2) & 0x3c];
			*p_dst++ = '=';
		}
		else if(p_end-p==1)
		{
			*p_dst++ = base64_index[(p[1] << 4) & 0x30];
			*p_dst++ = '=';
			*p_dst++ = '=';
		}
		*p_dst = '\0';
		return output;
	}
}


char* base64_encode_v2(char* binData,char*base64,int binLength)
{
	int i=0;
	int j=0;
	int current=0;
	for(i=0;i<binLength;i+=3)
	{
		current=(*(binData) >> 2) & 0x3f;
		*(base64+j++)=base64_index[current];
		current=(*(binData+i)<<4) & 0x30;
		if(binLength<=(i+1))
		{
			*(base64+j++)=base64_index[current];
			*(base64+j++)='=';
			*(base64+j++)='=';
			break;
		}
		current|=(*(binData+i+1)>>4) & 0xf;
		*(base64+j++)=base64_index[current];
		current=(*(binData+i+1)<<2) & 0x3c;
		if(binLength<=(i+2))
		{
			*(base64+j++)=base64_index[current];
			*(base64+j++)='=';
			break;
		}
		current|=(*(binData+i+2)>>6)&0x03;
		*(base64+j++)=base64_index[current];
		current=*(binData+i+2)&0x3f;
		*(base64+j++)=base64_index[current];
	}
	*(base64+j)='\0';
	return base64;
}
/*
void main()
{
	char *origin = "are you sb";
	char after[128];
	//base64_encode(origin,strlen(origin),after);
	base64_encode_v2(origin,after,strlen(origin));
	printf("after encrypt:%s\n",after);
	return;
}
*/
