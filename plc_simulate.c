/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\plc_simulate.c
#       @date         :2015-12-02 15:59
#       @algorithm    :
==========================================================================*/

#include "plc_simulate.h"
#include <netinet/in.h>
#include "rmfsystem.h"
#include <math.h>
#include <pthread.h>

int plc_float[15];

void * udp_send(void *arg)
{
	int i;
	unsigned char d[2];
	unsigned char  udp_buff[UDP_BUFF];
	int udp_socket;
	int client_socket_fd;
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=inet_addr(LOCAL_ADDRESS);
	server_addr.sin_port=htons(UDP_PORT);
	client_socket_fd=socket(AF_INET,SOCK_DGRAM,0);
	if(client_socket_fd<0)
	{
		perror("create socket error:");
		exit(1);
	}
    srand((unsigned)time(NULL));
	while(1)
	{
		for(i=0;i<2;i++)
		{
			d[i]=(unsigned char)rand()%256;
		}
		memcpy(udp_buff,d,UDP_BUFF);
		if(sendto(client_socket_fd,udp_buff,UDP_BUFF,0,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
		{
			perror("send udp data error:");
			break;
		}
		sleep(UDP_INTERVAL);
	}
	close(client_socket_fd);
	exit(1);

}

void *tcp_send(void*arg)
{
	int pid;
	int i=0;
	int tcp_socket;
	int a=1;
	int len;
	int fd;
	struct sockaddr_in address,client_addr;
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0)
	{
		perror("tcp connect error:");
		exit(1);
	}
	address.sin_family=AF_INET;
	address.sin_port=htons(TCP_PORT);
	address.sin_addr.s_addr=INADDR_ANY;
	bzero(&(address.sin_zero),8);
	struct fetch fetch_struct;
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&a,sizeof(int))==-1)
	{
		perror("sersocket error");
		exit(1);
	}
	if(bind(fd,(struct sockaddr*)&address,sizeof(struct sockaddr))<0)
	{
		perror("bind error:");
		exit(1);
	}
	if(listen(fd,5)==-1)
	{
		perror("listen error:");
		exit(1);
	}
	len=sizeof(client_addr);
	while(1)
	{
		tcp_socket=accept(fd,(struct sockaddr*)&client_addr,&len);
		if(tcp_socket==-1)
		{
			perror("accept error:");
			exit(1);
		}
		pid=fork();
		switch(pid)
        {
			case -1:
				printf("fork error\n");
				exit(1);
			case 0:
				while(1)
				{
					while(i<15)
					{
						plc_float[i++]=90;
					}
					if(read(tcp_socket,&fetch_struct,16)<0)
					{
						perror("read error:");
						exit(1);
					}
					else
					{
						printf("haha%02x\n",fetch_struct.org_id);
						if(fetch_struct.org_id==0x02)
						{
							response_data(fetch_struct,tcp_socket);
							printf("1\n");
						}
						else
							response_error(tcp_socket);
					}
				}
				break;
			default:
			{
				close(tcp_socket);
				break;
			}
		}
	}
}

void response_data(struct fetch fetch_data,int fd)
{
	int m;
//	printf("1\n");
	int i=0;
	float real[15];
	while(i<15)
	{
		real[i]=(float)(i+1)*(1+sin(ANGLE(plc_float[i])*PI/180));
		i++;
	}
	struct fetch_res res_struct,*p;
	p=&res_struct;
	init_struct(p);
	m=combine(fetch_data.len_h,fetch_data.len_l);
	unsigned char data[2048];
//	printf("m=%d i=%2x\n",m,fetch_data.dbnr);
	memcpy(data,real,m);
	write(fd,p,16);
	write(fd,data,m);
	printf("send ok second\n%02x\n",data[0]);
	return;
}

int combine(unsigned char a,unsigned char b)
{
	int m=(int)(b)+(int)a*16*16;
	return m;
}

void response_error(int fd)
{
	struct fetch_res res,*p;
	p=&res;
	init_struct(p);
	p->error_field=0x01;
	write(fd,p,16);
	return;
}

void init_struct(struct fetch_res *res_struct)
{
	res_struct->systemid_1=0x53;
	res_struct->systemid_2=0x35;
	res_struct->len_of_head=0x10;
	res_struct->id_op_code=0x01;
	res_struct->len_op_code=0x03;
	res_struct->op_code=0x06;
	res_struct->ack_field=0x0f;
	res_struct->len_ack_field=0x03;
	res_struct->error_field=0x00;
	res_struct->empty_field=0xff;
	res_struct->len_empty_field=0x07;
	memset(res_struct->fill_field,0x00,5);
	
}

int tcp_establish()
{
	int a=1;
	int len;
	int fd;
	int tcp_socket;
	struct sockaddr_in address,client_addr;
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0)
	{
		perror("tcp connect error:");
		exit(1);
	}
	address.sin_family=AF_INET;
	address.sin_port=htons(TCP_PORT);
	address.sin_addr.s_addr=INADDR_ANY;
	bzero(&(address.sin_zero),8);
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&a,sizeof(int))==-1)
	{
		perror("sersocket error");
		exit(1);
	}
	if(bind(fd,(struct sockaddr*)&address,sizeof(struct sockaddr))<0)
	{
		perror("bind error:");
		exit(1);
	}
	if(listen(fd,1)==-1)
	{
		perror("listen error:");
		exit(1);
	}
	len=sizeof(client_addr);
	tcp_socket=accept(fd,(struct sockaddr*)&client_addr,&len);
	if(tcp_socket==-1)
	{
		perror("accept error:");
		exit(1);
	}
//	printf("fd=%d\ni",tcp_socket);
	return tcp_socket;
}
void main()
{
	int err;
	pthread_t tid1;
	pthread_t tid2;
	pthread_attr_t attr;
	err=pthread_attr_init(&attr);
	if(err!=0)
		exit(1);
	err=pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
//	printf("1\n");
	if(err==0)
	{
//		printf("1\n");
		err=pthread_create(&tid1,&attr,udp_send,NULL);
		if(err!=0)
			exit(1);
		err=pthread_create(&tid2,&attr,tcp_send,NULL);
		if(err!=0)
			exit(1);
		pthread_attr_destroy(&attr);
	}
	pthread_exit((void*)1);
}
