/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\scan.c
#       @date         :2015/10/25 18:58
#       @algorithm    :
==========================================================================*/
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/select.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include"read_file.h"
#include"change_profile.h"

#define TCP_PORT 2000
#define MAXSIZE 100

int sock_connect(char*);
int scanip(void);
int scan(void);

int sock_connect(char *address)
{
	int flag;
	struct sockaddr_in servaddr;
	int  sockfd;
	fd_set west;
	int maxfd;
	int error;
	struct timeval timeout;
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		printf("create scoket error\n");
		exit(1);
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(TCP_PORT);
	int val=fcntl(sockfd,F_GETFL,0);
	fcntl(sockfd,F_SETFL,val| O_NONBLOCK);
//	printf("%s\n",address);
	servaddr.sin_addr.s_addr=inet_addr(address);
	int connect_flag=connect(sockfd,(struct sockaddr*)&servaddr,sizeof(struct sockaddr_in));
	if(connect_flag>=0)
	{
		return sockfd;
	}
	else if(errno==EINPROGRESS)
	{
		timeout.tv_sec=0;
		timeout.tv_usec=1000*200;
//		FD_ZERO(&rest);
		FD_ZERO(&west);
//		FD_SET(sockfd,&rest);
		FD_SET(sockfd,&west);
		maxfd=sockfd+1;
		flag=select(maxfd,NULL,&west,NULL,&timeout);
		if(flag<0)
		{
			close(sockfd);
			printf("select error\n");
			return 0;
		}
		if(FD_ISSET(sockfd,&west))
		{
			socklen_t len=sizeof(error);
			if(getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&len)<0)
			{
				close(sockfd);
				return 0;
			}
			if(error!=0)
			{
				return 0;
			}
			else 
			{
				return sockfd;
			}
		}
		else
		{
			return 0;
		}
	}
	else 
	{
		printf("connect error\n");
		close(sockfd);
		return 0;
	}
}


int scanip()
{
	int sockfd,n,length,maxfd,flag;
	int i=1;
	int my;
	int rc;
	char address[20];
	char temp[30];
	struct timeval timeout;
	fd_set rest,west;
	struct sockaddr_in servaddr,peersock;
	length=sizeof(peersock);
	int  error=0;
	read_file("PLC_INFO","address",address);
	printf("address=%s\n",address);
	if(strlen(address)>7)
	{
		rc=sock_connect(address);
		if(rc>0)
			goto done;
	}
	/*quite important!timeout must be set in the loop
	 because every loop will flush  the structure*/
//	timeout.tv_sec=2;
//	timeout.tv_usec=0;
	printf("scan begin!~~~~~~~~~~~\n");	
	for(;i<255;i++)
	{
		sprintf(address,"192.168.1.%d",i);
		printf("%s\n",address);
		rc=sock_connect(address);
		if(rc>0)
		{
			printf("1\n");
			goto done;
		}
	}
done:
	if(i<254)
	{
		flag=fcntl(sockfd,F_GETFL,0);
		fcntl(rc,F_SETFL,flag&~O_NONBLOCK);
		printf("scan successfully!\naddr:%s\nport:%d\n",address,TCP_PORT);
		sprintf(temp,"address=%s",address);
		addoraltconfig("./device.config","address",temp);
		return rc;
	}
	else
		return 0;
}


int scan()
{
	int sockfd;
	while(1)
	{
	
		sockfd=scanip();
		if(sockfd==0)
		{
			printf("cannot match\n");
		}
		else 
		{
			printf("scan success\n");
			break;
		}
		sleep(60);
	}
	return 0;

}
