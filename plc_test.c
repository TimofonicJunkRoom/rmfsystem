/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\plc_test.c
#       @date         :2015/10/29 12:56
#       @algorithm    :
==========================================================================*/

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include "plc_simulate.h"
#include "encrpyt.h"

#define TCP_ADDR "192.168.0.1"
#define TCP_PORT 2000
#define UDP_LEN 100
#define UDP_PORT 10000
#define PUBLIC_KEY "public.key"
#define PUBLIC_TXT "public.txt"

int tcp_connect(void);
void udp_communication(void);
void tcp_encrypt();

void tcp_encrypt()
{
	char np[4096];
	char bp[4096];
	int n;
	FILE*fp;
	char *content;
	char temp[4096];
	int sock;
	fp=fopen(PUBLIC_KEY,"w");
	sock=tcp_connect();
	n=read(sock,temp,4096);
	printf("%d\n",n);
	strcpy(np,"-----BEGIN PUBLIC KEY-----\n");
	strcat(np,temp);
	strcat(np,"\n-----END PUBLIC KEY-----");
	printf("%s\n",np);
	fputs(np,fp);
	fclose(fp);
	content=encrypt("are you sb?",PUBLIC_KEY);
	write(sock,content,4096);
	close(sock);
	return;
	
}

int tcp_connect()
{
	int client_sockfd;
	int client_len;
	struct sockaddr_in client_address;
	int result,i=0;
	client_sockfd=socket(AF_INET,SOCK_STREAM,0);
	client_address.sin_family=AF_INET;
	client_address.sin_addr.s_addr=inet_addr(TCP_ADDR);
	client_address.sin_port=htons(TCP_PORT);
	client_len=sizeof(client_address);
	result=connect(client_sockfd,(struct sockaddr*)&client_address,client_len);
	if(result==-1)
	{
		perror("connect error:");
		exit(1);
	}
	return client_sockfd;
}


void tcp_communication()
{
	int i;
	int rc;
	int sockfd;
	struct fetch request;
	unsigned char buff[200];
	sockfd=tcp_connect();
	request.systemid_1=0x53;
	request.systemid_2=0x35;
	request.len_of_head=0x10;
	request.id_op_code=0x01;
	request.len_op_code=0x03;
	request.op_code=0x05;
	request.org_field=0x03;
	request.len_org_field=0x08;
	request.empty_field=0xff;
	request.len_empty_field=0x02;
	/*need fill in*/
	request.org_id=0x02;
	request.dbnr=0x00;
	request.start_address_h=0x00;
	request.start_address_l=0x00;
	request.len_h=0x00;
	request.len_l=0x02;
	rc=write(sockfd,&request,sizeof(struct fetch));
	if(rc<16)
	{
		printf("send error\n");
		exit(1);
	}
	rc=read(sockfd,buff,16);
	if(rc<16)
	{
		printf("tcp recv error\n");
		exit(1);
	}
	rc=read(sockfd,buff,200);
	if(rc<=0)
	{
		printf("tcp recv error\n");
		exit(1);
	}
	else
	{
		for(i=0;i<rc;i++)
		{
			printf("%02x\t",buff[i]);
			if(i%5==0)
				printf("\n");
		}
		printf("\nrecv over\n");
	}
}

void udp_communication()
{
	int i;
	int udp_socket;
	struct sockaddr_in server;
	struct sockaddr_in client;
	unsigned char buff[UDP_LEN];
	socklen_t sin_size;
	int num;
	udp_socket=socket(AF_INET,SOCK_DGRAM,0);
	bzero(&server,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(UDP_PORT);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	bind(udp_socket,(struct sockaddr*)&server,sizeof(struct sockaddr));
	sin_size=sizeof(struct sockaddr_in);
	while(1)
	{
		num=recvfrom(udp_socket,buff,UDP_LEN,0,(struct sockaddr*)&client,&sin_size);
		if(num<0)
		{
			perror("recv error:");
			exit(1);
		}
		else
		{
			printf("recv udp\n");
			for(i=0;i<num;i++)
			{
				printf("%02x\t",buff[i]);
				if(i%5==0)
					printf("\n");
			}
			printf("\nrecv over\n");
		}
	}
}

void main()
{
//	udp_communication();
//	tcp_encrypt();
	tcp_communication();
}
