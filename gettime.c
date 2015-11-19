/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\gettime.c
#       @date         :2015/11/19 23:44
#       @algorithm    :
==========================================================================*/

#include <stdio.h>
#include <sys/time.h>
#include <time.h>

int main()
{
	struct timeval tv;
	while(1)
	{
		gettimeofday(&tv,0);
		printf("time %u:%u\n",(unsigned int)tv.tv_sec,(unsigned int)tv.tv_usec);
		sleep(2);
	}
	return 0;
}
