/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/haha\main.c
#       @date         :2015-07-07 13:42
#       @algorithm    :
==========================================================================*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

pid_t com=-1;
pid_t deal=-1;
pid_t local=-1;

int main()
{	
	int rc;
	int status;
	pid_t temp;
	while(1)
	{
		if(com==-1)
		{
			temp=fork();
			com=temp;
			if(temp<0)
				printf("error in com\n");
			else if(temp==0)
				if((rc=execl("./comtest_pipe_em","comtest_pipe_em",NULL))<0)
				{
					printf("main com err\n");
				}
		}
		sleep(2);
		if(deal==-1)
		{
			temp=fork();
			deal=temp;
			if(temp<0)
				printf("error in com\n");
			else if(temp==0)
				if((rc=execl("./data_deal_em","data_deal_em",NULL))<0)
				{
					printf("main deal err:%d\n",errno);
				}
		}
		sleep(2);
		if(local==-1)
		{
			temp=fork();
			local=temp;
			if(temp<0)
				printf("error in com\n");
			else if(temp==0)
				if((rc=execl("./client_pipe_em","client_pipe_em",NULL))<0)
				{
					printf("main local err\n");
				}
		}
		temp=waitpid(-1,&status,0);
		if(temp==com)
		{
			com=-1;
			kill(temp,SIGKILL);
			printf("main com error\n");
		}
		if(temp==deal)
		{
			deal=-1;
			kill(temp,SIGKILL);
			printf("main deal error\n");
		}
		if(temp==local)
		{
			local=-1;
			kill(temp,SIGKILL);
			printf("main local error\n");
		}
	}
}
