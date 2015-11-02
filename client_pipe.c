/*=========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\client_pipe.c
#       @date         :2015-10-07 19:38
#       @algorithm    :
=========================================================================*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <limits.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "rmfsystem.h"
#include "shm_mem.h"
#include <pthread.h>
#include "change_profile.h"
#include "read_file.h"
#include <sys/file.h>
#include <netdb.h>
#include <sys/wait.h>
#include <asm/ioctls.h>

#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...)  printf("File: "__FILE__",Line: %04d: "format"\n",__LINE__,##__VA_ARGS__)
#else
#define DEBUG(format,...)
#endif
#define HOST_ADDRESS "10.60.140.246"
//#define HOST_ADDRESS "192.168.42.21"
#define REAL_TIME "real_time.xml"
#define FIFO_NAME "/tmp/my_fifo"

int commandnumber;
int datalevel1;
int datalevel2;
int confirm=0;
int real_time=0;
int conn();
int update_data_1(int);
int update_data_2(int);
int first_login();
int login();
int get_line(int sock,char *buf,int size);
int change_file_single_value(char*buf);
int change_file(char*buf);
int change_config(char*,char *);
int recv_fork();
int insert_command(int,char*,char*,char*);
int send_fork();
void handler();
void send_data();
void gettime(char*);
void msg_init(void);
int msg_recv(struct msg_remote*);
void * remote_recv();
void * remote_send();
int device_change();
int real_time_data();
int recv_error();
void dbinit();
int send_signal();
int msg_send(struct msg_remote*);
int send_signal_usr2();

int socketfd;
sqlite3*db;
//int count;
char username[100];
int remote_msgid;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;

void write_pid_remote();

void write_pid_remote()
{
	char line[50];
	char newline[50];
	int pid;
	FILE*fp;
	FILE*fpnew;
	fp=fopen(PRO_CONF,"r+");

	if(fp==NULL)
		exit(1);
	fpnew=fopen("process.config.new","a");
	if(fpnew==NULL)
		exit(1);
	while(fgets(line,50,fp))
	{
		if(strncmp(line,"remote_pid",10)==0)
		{
			pid=getpid();
			sprintf(newline,"remote_pid=%d\n",pid);
			fputs(line,fpnew);
		}
		else
			fputs(line,fpnew);
	}
	fclose(fp);
	fclose(fpnew);
	remove(PRO_CONF);
	rename("process.config.new",PRO_CONF);
//	printf("write process.config ok\n");
	return;
}

int send_signal()
{
	int ret;
	int pid;
	char *p;
	char line[100];
	FILE*fp;
	if((fp=fopen(PRO_CONF,"r"))==NULL)
	{
		DEBUG("OPEN ERROR");
		return 0;
	}
	while((fgets(line,sizeof(line),fp))!=NULL)
	{
		if(strncmp(line,"local_pid",9)==0)
		{
			p=strchr(line,'=');
			p++;
			line[strlen(line)-1]='\0';
			pid=atoi(p);
		}
	}
	ret=kill((pid_t)pid,SIGUSR1);
	if(ret==0)
		return 1;
	else 
		return 0;
}

int data_response()
{
	int rc;
	int level;
	int number;
	char*p;
//	printf("data response\n");
	char response[50];
	while(1)
	{
		rc=get_line(socketfd,response,50);
		if(rc>1)
		{
			if(strncmp(response,"datalevel",9)==0)
			{
				p=strchr(response,'=');
				p++;
				response[strlen(response)-1]='\0';
				level=atoi(p);
			}
			if(strncmp(response,"number",6)==0)
			{
				p=strchr(response,'=');
				p++;
				response[strlen(response)-1]='\0';
				number=atoi(p);
			}
		}
		else
			break;
	}
//	printf("level=%d,number=%d\n",level,number);
	if((level==1&&number==datalevel1)|(level==2&&number==datalevel2))
	{
		pthread_mutex_lock(&mutex);
		confirm=1;
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond);
	}
	if(level==1)
		update_data_1(number);
	else if(level==2)
		update_data_2(number);
	else
		DEBUG("UPDATE ERROR\n");
//	printf("10\n");
	return 1;

}

int update_data_1(int number)
{
	int rc;
//	printf("number=%d\n",number);
	sqlite3_stmt * stmt3=NULL;
	char * sql="update device_first_level_data set confirm=1 where number=?";
	while((rc=sqlite3_prepare_v2(db,sql,strlen(sql),&stmt3,NULL))!=SQLITE_OK)
	{
		printf("tata\n");
		if(rc==SQLITE_BUSY)
		{
			usleep(10000);
			continue;
		}
		if(stmt3)
			sqlite3_finalize(stmt3);
		sqlite3_close(db);
		DEBUG("PREPARE ERROR!");
		exit(1);
	}
	sqlite3_bind_int(stmt3,1,number);
	while((rc=sqlite3_step(stmt3))!=SQLITE_DONE)
	{
		if(rc==SQLITE_BUSY)
		{
			usleep(10000);
			continue;
		}
		sqlite3_finalize(stmt3);
		sqlite3_close(db);
		DEBUG("STEP ERROR!");
		exit(1);
	}
	sqlite3_reset(stmt3);
}

int update_data_2(int number)
{
	int rc;
//	printf("number=%d\n",number);
	sqlite3_stmt * stmt3=NULL;
	const char * sql="update device_second_level_data set confirm=1 where number=?";
	while((rc=sqlite3_prepare_v2(db,sql,strlen(sql),&stmt3,NULL))!=SQLITE_OK)
	{
		if(rc==SQLITE_BUSY)
		{
			usleep(10000);
			continue;
		}
		if(stmt3)
			sqlite3_finalize(stmt3);
	//	sqlite3_close(db);
		DEBUG("PREPARE ERROR:%s",sqlite3_errmsg(db));
		exit(1);
	}
	sqlite3_bind_int(stmt3,1,number);
	while((rc=sqlite3_step(stmt3))!=SQLITE_DONE)
	{
		if(rc==SQLITE_BUSY)
		{
			usleep(10000);
			continue;
		}
		sqlite3_finalize(stmt3);
	//	sqlite3_close(db);
		DEBUG("STEP ERROR:%s",sqlite3_errmsg(db));
		exit(1);
	}
	sqlite3_reset(stmt3);
}

/*
int update_data_2(int number)
{
	sqlite3_stmt * stmt;
	printf("number=%d\n",number);
	const char *sql1="update device_second_level_data set confirm=1 where number=?";
	if(sqlite3_prepare_v2(db,sql1,strlen(sql1),&stmt,NULL)!=SQLITE_OK)
	{
		if(stmt)
			sqlite3_finalize(stmt);
		sqlite3_close(db);
		DEBUG("PREPARE ERROR!");
		exit(1);
	}
	sqlite3_bind_int(stmt,1,number);
	if(sqlite3_step(stmt)!=SQLITE_DONE)
	{
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		DEBUG("STEP ERROR:%d",errno);
		exit(1);
	}
	sqlite3_reset(stmt);
}
*/
/*int update_data(int datalevel,int number)
{
	int rc;
	char sql[100];
	char *errmsg=0;
	if(datalevel==1)
	{
		sprintf(sql,"update device_first_level_data set confirm=1 where number=%d",number);
		while(1)
		{
			rc=sqlite3_exec(db,sql,NULL,NULL,&errmsg);
			if(rc!=SQLITE_OK)
			{
				if(rc==SQLITE_BUSY)
					continue;
				DEBUG("ERRMSG=%s",errmsg);
				sqlite3_free(errmsg);
				perror("update error:");
				return 0;
			}
			else
				break;
		}
	}
	else if(datalevel==2)
	{
		sprintf(sql,"update device_second_level_data set confirm=1 where number=%d",number);
		while(1)
		{
			rc=sqlite3_exec(db,sql,NULL,NULL,&errmsg);
			if(rc!=SQLITE_OK)
			{
				if(rc==SQLITE_BUSY)
					continue;
				DEBUG("UPDATE ERROR");
				sqlite3_free(errmsg);
				return 0;
			}
			else 
				break;
		}
	}
	else
		return 0;
	return 1;
}
*/
int device_change()
{
	int n;
	int rc;
	char*p;
	char command[5]="CHAG";
	char time[50];
	char recvbuff[50];
	char commandline[50];
	char parameter[20];
	char value[20];
	char number[10];
	struct msg_remote msg_data;
	while(1) 
	{
		rc=get_line(socketfd,recvbuff,sizeof(recvbuff));
		if(rc>1)
		{
			if(strncmp(recvbuff,"parameter",9)==0)
			{
				p=strchr(recvbuff,'=');
				p++;
				strcpy(parameter,p);
				p=strchr(parameter,'\n');
				*p='\0';
			}
			if(strncmp(recvbuff,"value",5)==0)
			{
				p=strchr(recvbuff,'=');
				p++;
				strcpy(value,p);
				p=strchr(value,'\n');
				*p='\0';
			}
			if(strncmp(recvbuff,"number",6)==0)
			{
				p=strchr(recvbuff,'=');
				p++;
				strcpy(number,p);
				p=strchr(number,'\n');
				*p='\0';
			}
		}
		else
			break;
	}
	n=atoi(number);
	commandnumber=n;
//	printf("n=%d\n",n);
	sprintf(commandline,"%s=%s",parameter,value);
	printf("CHAG: %s\n",commandline);
	gettime(time);
	insert_command(n,time,command,commandline);
	addoraltconfig(DEV_CONF,parameter,commandline);
	send_signal();
	msg_data.mtype=1;
	strcpy(msg_data.command,"2120");
	msg_data.number=commandnumber;
	strcpy(msg_data.time,time);
	msg_send(&msg_data);
	return 1;
}

int msg_send(struct msg_remote*data)
{
	if(msgsnd(remote_msgid,(void*)data,sizeof(struct msg_remote),0)==-1)
	{
		DEBUG("REMOTE MSGSND ERROR");
		return 0;
	}
	else
		return 1;
}

int insert_command(int number,char *time,char *command,char*context)
{
	int rc;
	char sql[200];
	char *errmsg=0;
	sprintf(sql,"insert into device_command values(%d,\"%s\",\"%s\",\"%s\",1)",number,time,command,context);
	while(1)
	{
		rc=sqlite3_exec(db,sql,0,0,&errmsg);
		if(rc!=SQLITE_OK)
		{
			if(rc==SQLITE_BUSY)
				continue;
			DEBUG("REMOTE SQLITE3 INSERT");
			exit(1);
		}
		else
			break;
	}
	return 1;
}

void dbinit()
{
	db=NULL;
	char *errmsg;
	int rc;
	rc=sqlite3_open("local.db",&db);
	if(rc!=SQLITE_OK)
	{
		DEBUG("CANnot OPEN DB:%s",sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	else
		printf("remote open local.db successfully\n");
	return;
}

void msg_init(void)
{
	int msgid;
	msgid=msgget((key_t)MSGID_REMOTE,0666|IPC_CREAT);
	if(msgid==-1)
	{
		DEBUG("LOCAL MSG ERROR");
		exit(EXIT_FAILURE);
	}
	printf("local_msg init ok\n");
	remote_msgid=msgid;
	return;
}

int msg_recv(struct msg_remote*data)
{	
	int ret;
	int length;
	length=2*sizeof(struct msg_remote);
	if((ret=msgrcv(remote_msgid,(void*)data,length,0,IPC_NOWAIT))==-1)
	{
		if(errno!=ENOMSG)	
		{
			DEBUG("LOCAL MSGRCV ERROR");
			exit(1);
		}
		return 0;
	}
	else
	{
//		printf("length=%d\n",data->length);
		return 1;
	}
}

void * remote_recv()
{
	printf("remote secv pthread begin\n");
	char recvbuff[100];
	int rc;
	while(1)
	{
		rc=get_line(socketfd,recvbuff,sizeof(recvbuff));
//		printf("recvbuff=%s\n",recvbuff);
		if(strncmp(recvbuff,"CHAG",4)==0)
		{
			printf("change profile\n");
			device_change();		
		}
		else if(strncmp(recvbuff,"RETM",4)==0)
		{
			printf("real time data begin!\n");
			rc=real_time_data();
		}
		else if(strncmp(recvbuff,"2200",4)==0)
		{
			data_response();
		}
		else if(strncmp(recvbuff,"NOOP",4)==0|strncmp(recvbuff,"4100",4)==0)
		{
			while(get_line(socketfd,recvbuff,sizeof(recvbuff))>1);
		}
		else 
		{
			rc=recv_error();
		}
	}
}

int recv_error()
{
	struct msg_remote errdata;
	errdata.mtype=1;
	char buff[50];
	char time[100];
	char *p;
	char err[4]="err";
	int number=0;
	while(get_line(socketfd,buff,50)>1)
	{
		if(strncmp(buff,"number",6)==0)
		{
			p=strchr(buff,'=');
			p++;
			buff[strlen(buff)-1]='\0';
			number=atoi(p);
			errdata.number=number;
		}
	}
	if(number<=0)
		errdata.number=0;
	gettime(time);
	strcpy(errdata.time,time);
	sprintf(errdata.command,"3300");
	insert_command(errdata.number,time,err,err);
	msg_send(&errdata);
}

int real_time_data()
{
	int i;
	char n[10]; 
	int local_pid;
	int deal_pid;
	int rc;
	char *p;
	int  parameter;
	char buff[50];
	char temp[50];
	char c;
	char d;
	read_file("real_time","real_time_data",temp);
	d=(char)(atoi(temp)+48);
//	printf("d=%c\n",d);
	while(1)
	{
		rc=get_line(socketfd,buff,50);
		if(rc>1)
		{
			if(strncmp(buff,"begin",5)==0)
			{
				p=strchr(buff,'=');
				p++;
				buff[strlen(buff)-1]='\0';
				c=*p;
			}
			if(strncmp(buff,"number",6)==0)
			{
				p=strchr(buff,'=');
				p++;
				strcpy(n,p);
				n[strlen(n)-1]='\0';
				commandnumber=atoi(n);
			}
			if(strncmp(buff,"parameter",9)==0)
			{
				p=strchr(buff,'=');
				p++;
				buff[strlen(buff)-1]='\0';
				parameter=atoi(p);
			}
		}
		else 
			break;
	}
//	printf("parameter=%d\n",parameter);
	if(c!=d)
	{
		real_time=(real_time?0:1);
		if(c>0)
		{
			pthread_mutex_lock(&mutex);
			confirm=1;
			pthread_mutex_unlock(&mutex);
			pthread_cond_signal(&cond);
		}
	//	printf("1\n");
		fflush(stdout);
		printf("real_time=%d\n",real_time);
		fflush(stdout);
		if(real_time)
		{
			sprintf(temp,"real_time_data=%d",parameter);
		}
		else
		{
			sprintf(temp,"real_time_data=%d",0);
		}
		addoraltconfig(DEV_CONF,"real_time_data",temp);
		send_signal_usr2();
	}
	return 1;
}

int send_signal_usr2()
{
	int ret;
	int local_pid;
	int deal_pid;
	char *p;
	char line[100];
	FILE*fp;
	if((fp=fopen(PRO_CONF,"r"))==NULL)
	{
		printf("open process.config error\n");
		return 0;
	}
	while((fgets(line,sizeof(line),fp))!=NULL)
	{
		if(strncmp(line,"local_pid",9)==0)
		{
			p=strchr(line,'=');
			p++;
			line[strlen(line)-1]='\0';
			local_pid=atoi(p);
		}
		if(strncmp(line,"deal_pid",8)==0)
		{
			p=strchr(line,'=');
			p++;
			line[strlen(line)-1]='\0';
			deal_pid=atoi(p);
		}
	}
//	printf("local=%d,deal=%d\n",local_pid,deal_pid);
	ret=kill((pid_t)local_pid,SIGUSR2);
	if(ret!=0)
		return 0;
	ret=kill((pid_t)deal_pid,SIGUSR2);
	if(ret!=0)
		return 0;
	printf("send signal ok\n");
	return 1;
}

void main()
{
	int err;
	dbinit();
	msg_init();
	write_pid_remote();
	pthread_t tid1;
	pthread_t tid2;
	pthread_attr_t attr;
	err=pthread_attr_init(&attr);
	if(err!=0)
	{
		DEBUG("ERROR");
		exit(1);
	}
	err=pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	printf("local begin\n");
	socketfd=conn();
	if(err==0)
	{
		err=pthread_create(&tid1,&attr,remote_recv,NULL);
		if(err!=0)
			exit(1);
		err=pthread_create(&tid2,&attr,remote_send,NULL);
		if(err!=0)
			exit(1);
		pthread_attr_destroy(&attr);
	}
	pthread_exit((void*)0);
/*
	char buffer[512]="username=okc\n";
	change_file(buffer);
	return;
*/
}

int conn()
{
	int fd;
	char*p;
	FILE *fp;
	char temp[4096];
	fp=fopen(CONFIG,"r");
	fgets(temp,4096,fp);
	fclose(fp);
	if(strncmp(temp,"init",4)==0)
	{
		p=strchr(temp,'=');
		p++;
//		printf("1\n");
		if(strncmp(p,"1",1)!=0)
		{
			first_login();
		}	
		fd=login();
	}
	printf("conn ok\n");
	return fd;
}

int first_login()
{
	struct timeval timeo;
	timeo.tv_sec=3;
	timeo.tv_usec=0;
	int rc;
	int fd;
	int len,i=0,n=0;
	struct sockaddr_in address;
	int result;
	FILE *fp;
	char *p,ptr[4096];
	char write_buf[4096],read_buf[4096],file_buf[4096],temp[4096];
	fd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(HOST_ADDRESS);
	address.sin_port = htons(13000);
	len = sizeof(address);
	setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&timeo,sizeof(struct timeval));
	result = connect(fd, (struct sockaddr *)&address, len);
	while(i<=20)
	{	
		if(result!=-1)
			break;
		result = connect(fd, (struct sockaddr *)&address, len);
		sleep(++i*3);
	}
	if(result == -1) {
		DEBUG("CONNECT ERROR");
		exit(1);
	}
	printf("local_first connection\n");
	snprintf(write_buf,4096,"RCON 002\r\nlength=0\r\n\r\n");
	write(fd,write_buf,strlen(write_buf));
//	printf("send ok**************\n%s\n",write_buf);
	n=get_line(fd,read_buf,4096);
//	printf("recv ok**************\n%s\n",read_buf);
	if(n<=0)
	{
		perror("local_read error\n");
		exit(1);
	}
	if(strncmp(read_buf,"1010",4)!=0)//?
	{
		perror("local_RCON error");
		exit(1);
	}
	while(get_line(fd,read_buf,4096)>1);
	snprintf(write_buf,4096,"LOGI 002\r\n");
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
//	printf("send ok**************\n%s\n",write_buf);
	//	write_buf="LOGI 002\r\n";
	fp=fopen(CONFIG,"r+");
	fgets(temp,4096,fp);
	fgets(temp,4096,fp);
	memcpy(write_buf,temp,4096); //write_buf=temp;
	fgets(temp,4096,fp);//!
	strcat(write_buf,temp);//!
	if(write_buf[strlen(write_buf)-1]=='\n')
		write_buf[strlen(write_buf)-1]='\0';
	strcat(write_buf,"\r\nlength=0\r\n\r\n");
	
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
//	printf("send ok**************\n%s\n",write_buf);
	n=get_line(fd,read_buf,4096);
//	printf("recv ok**************\n%s\n",read_buf);
	if(strncmp(read_buf,"1020",4)!=0)
	{
		DEBUG("LOGIN ERROR");
		exit(1);
	}
	while(get_line(fd,read_buf,4096)>1);
	//write_buf="LGIN 002\r\n";
	snprintf(write_buf,4096,"LGIN 002\r\n");
	while(1)
	{
		fgets(temp,4096,fp);
		if(!feof(fp))
		{
			strcat(write_buf,temp);
		}
		else
			break;
	}
	fclose(fp);
	strcat(write_buf,"\r\nlength=0\r\n\r\n");
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
//	printf("send ok***************\n%s\n",write_buf);
	get_line(fd,read_buf,4096);
//	printf("recv ok***************\n%s\n",read_buf);
	if(strncmp(read_buf,"2010 002",8)!=0)
	{
		DEBUG("LOCAL LOGIN ERROR");
		exit(1);
	}
	addoraltconfig("config","init","init=1");
	get_line(fd,read_buf,4096);
//	printf("recv ok**************\n%s\n",read_buf);
	change_file(read_buf);
	while(get_line(fd,temp,4096)>1)
	{
		change_file(temp);
//		printf("recv ok***************\n%s\n",temp);
	}
	//file_buf="1\r\n";
	/*
	strcat(file_buf,read_buf);
	for(p=file_buf,i=0;*p!='\0';p++)
		if(*p!='\r')
			ptr[i++]=*p;

	ptr[i-1]='\0';
	printf("%s\n",ptr);
	fp=fopen("config","w");
	fputs(ptr,fp);
	fclose(fp);
*/	//write_buf="QUIT 002\r\n";
	snprintf(write_buf,4096,"QUIT 002\r\nlength=0\r\n\r\n");
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
//	printf("send ok******************\n%s\n",write_buf);
	get_line(fd,read_buf,4096);
//	printf("recv ok******************\n%s\n",read_buf);
	if(strncmp(read_buf,"2000 002",8)!=0)
	{	
		DEBUG("LOCAL QUIT ERROR");
		exit(1);
	}
	while(get_line(fd,read_buf,4096)>1);
	close(fd);
	return 0;
}

int login()
{
	struct timeval timeo;
	int rc;
	int i=0;
	int len,length,result;	
	int fd;
	struct sockaddr_in address;
	FILE *fp;
	char *p,*ptr;
	timeo.tv_sec=5;
	timeo.tv_usec=0;
	char write_buf[4096],read_buf[4096],file_buf[4096],temp[4096];
	fd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(HOST_ADDRESS);
	address.sin_port = htons(13000);
	len = sizeof(address);
	setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&timeo,sizeof(struct timeval));
	result = connect(fd, (struct sockaddr *)&address, len);
	while(i<=20)
	{	
		if(result!=-1)
			break;
		result = connect(fd, (struct sockaddr *)&address, len);
		if(result==-1)
		{
			DEBUG("CONNECT ERROR:%d",errno);
		}
		sleep(++i*5);
	}
	if(result == -1) {
		DEBUG("CONNECT ERROR:%d",errno);
		exit(1);
	}
//	setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&timeo,sizeof(struct timeval));
	setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeo,sizeof(struct timeval));
//	int keepalive=1;
//	int keepidle=2;
//	int keepinternal=5;
//	int keepcount=3;
//	setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(void *)&keepalive,sizeof(keepalive));
//	setsockopt(fd,SOL_SOCKET,TCP_KEEPIDLE,(void *)&keepidle,sizeof(keepidle));
//	setsockopt(fd,SOL_SOCKET,TCP_KEEPINTVL,(void *)&keepinternal,sizeof(keepinternal));
//	setsockopt(fd,SOL_SOCKET,TCP_KEEPCNT,(void *)&keepcount,sizeof(keepcount));
	printf("local CONNECTION!\n");
	snprintf(write_buf,4096,"RCON 002\r\nlength=0\r\n\r\n");
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
//	printf("send ok****************\n%s\n",write_buf);
	get_line(fd,read_buf,4096);
//	printf("recv ok****************\n%s\n",read_buf);
	if(strncmp(read_buf,"1010",4)!=0)//?
	{
		DEBUG("LOCAL RCON ERROR");
		exit(1);
	}
	while(get_line(fd,read_buf,4096)>1);
	//write_buf="LOGI 002\r\n";
	snprintf(write_buf,4096,"LOGI 002\r\n");
	fp=fopen(CONFIG,"r+");
	fgets(temp,4096,fp);
	fgets(temp,4096,fp);
	p=strchr(temp,'=');
	p++;
	strcpy(username,p);
	p=strchr(username,'\n');
	*p='\0';
	strcat(write_buf,temp);//!
	fgets(temp,4096,fp);//!
	fclose(fp);
	strcat(write_buf,temp);//!
	write_buf[strlen(write_buf)-1]='\0';
	strcat(write_buf,"\r\nlength=0\r\n\r\n");
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
//	printf("send ok************\n%s\n",write_buf);
	get_line(fd,read_buf,4096);
//	printf("recv ok************\n%s\n",read_buf);
	if(strncmp(read_buf,"2000 002",8)!=0)
	{
		DEBUG("LOCAL LOGI ERROR");
		exit(1);
	}
	while(get_line(fd,read_buf,4096)>1);
	sprintf(write_buf,"PLCP 002\r\nlength=0\r\n\r\n");
	rc=write(fd,write_buf,strlen(write_buf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
	get_line(fd,read_buf,4096);
	if(strncmp(read_buf,"2010",4)!=0)
	{
		DEBUG("PLCP ERROR");
		exit(1);
	}
	while(get_line(fd,write_buf,4096)>1)
	{
		if(strncmp(write_buf,"length",6)==0)
		{
			p=strchr(write_buf,'=');
			p++;
			write_buf[strlen(write_buf)-1]='\0';
			len=atoi(p);
//			printf("len=%d\n",len);
		}
	}
	memset(read_buf,'\0',sizeof(read_buf));
	i=0;
	while(1)
	{
		
		length=read(fd,temp,4096);
//		printf("%d\n",length);
		if((length!=0)&&length<=len)
		{
//			printf("length=%d\n",length);
			memcpy(read_buf+i,temp,length);
			i=i+length;
			len=len-length;
		}
		if(len==0)
			break;
	}
//	printf("len=%d\n",len);
	fp=fopen(REAL_TIME,"w");
	fputs(read_buf,fp);
	fclose(fp);
	printf("local connection ok\n");
	return fd;
}

int get_line(int sock,char*buf,int size)
{
	int m;
	int i=0;
	char c='\0';
	int n;
	while((i<size-1)&&(c!='\n'))
	{
		n=recv(sock,&c,1,0);
		if(n>0)
		{

				if(c=='\r')
				{
					n=recv(sock,&c,1,MSG_PEEK);
				
					if((n>0)&&(c=='\n'))
					{
						m=recv(sock,&c,1,0);
						if(m<0)
						{
							DEBUG("ERROR");
							exit(1);
						}
					}
					else if(n==0)
					  c='\n';
					else
					{
						DEBUG("ERROR");
						exit(1);
					}
				}
				buf[i]=c;
				i++;
		}
		else if(n==0)
			c='\n';
		else
		{
			DEBUG("ERROR:%d",errno);
			exit(1);
		}
	}
	buf[i]='\0';
	return(i);
}

int change_file_single_value(char* buf)
{
	char linebuffer[512]={0};
	char buffer1[512]={0};
	char buffer2[512]={0};
	int line_len=0;
	int len=0,length=0;
	int res;
	char*p;
	p=strchr(buf,'=');
	length=p-buf;
	FILE *fp=fopen(CONFIG,"r+");
	if(fp==NULL)
	{
		DEBUG("LOCAL OPEN ERROR");
		exit(1);
	}
	while(fgets(linebuffer,512,fp))
	{
		line_len=strlen(linebuffer);
		len+=line_len;
		sscanf(linebuffer,"%[^=]=%[^=]",buffer1,buffer2);
		if(!strncmp(buf,buffer1,length))
		{
			len-=strlen(linebuffer);
			res=fseek(fp,len,SEEK_SET);
			if(res<0)
			{
				DEBUG("LOCAL FSEEK ERROR");
				exit(1);
			}
			memcpy(buffer1,buf,strlen(buf));
			fprintf(fp,"%s",buffer1);
			fclose(fp);
			return;
		}
	}
	return 0;
}

int change_file(char *buf)
{
	char linebuffer[512]={0};
	char buffer1[512]={0};
	char buffer2[512]={0};
	int line_len=0;
	int len=0,length=0;
	int res;
	char*p;
	p=strchr(buf,'=');
	length=p-buf;
	FILE *newfp=fopen("config.new","a");
	FILE *fp=fopen(CONFIG,"r+");
	if(fp==NULL)
	{
		printf("local_open error\n");
		exit(1);
	}
	while(fgets(linebuffer,512,fp))
	{
		if(strncmp(linebuffer,buf,length)!=0)
		{
			fputs(linebuffer,newfp);
		}
		else
			fputs(buf,newfp);		
	}
	fclose(fp);
	fclose(newfp);
	remove(CONFIG);
	rename("config.new",CONFIG);
	return 0;
}
/*	
int change_config(char *parameter,char* value)
{
	char linebuffer[512]={0};
	char newline[100];
	int len=0,length=0;
	char*p;
	length=strlen(parameter);
	sprintf(newline,"%s=%c\n",parameter,*value);
	printf("newline=%s\n",newline);
	FILE *newfp=fopen("device.config.new","a");
	FILE *fp=fopen(DEV_CONF,"r+");
	if(fp==NULL)
	{
		printf("remote_open error\n");
		exit(1);
	}
	while(fgets(linebuffer,512,fp))
	{
		if(strncmp(linebuffer,parameter,length)!=0)
		{
			fputs(linebuffer,newfp);
		}
		else
			fputs(newline,newfp);		
	}
	fclose(fp);
	fclose(newfp);
	remove(DEV_CONF);
	rename("device.config.new",DEV_CONF);
	return 0;
}

*/
int send_fork()
	{
		pid_t pid;
		pid=fork();
		if(pid<0)
	{
		perror("local fork error\n");
		exit(1);
	}
	else if(pid==0)
	{
/*		struct itimerval value,ovalue;
		signal(SIGALRM,handler);
		value.it_value.tv_sec=5;
		value.it_value.tv_usec=0;
		value.it_interval.tv_sec=5;
		value.it_interval.tv_usec=0;
		setitimer(ITIMER_REAL,&value,&ovalue);
*/
		while(1)
		{
		//	send_data();
			fflush(stdout);
		}
	}
	close(socketfd);
	return pid;
}

/*void handler()
{
	signal(SIGALRM,handler);
	send_data();
	return;
}
*/
int send_level1(struct msg_remote data)
{
	int rc;
	char sendbuf[512];
	sprintf(sendbuf,"%s 002\r\nlength=0\r\nnumber=%d\r\n\r\n",data.command,data.number);
	rc=write(socketfd,sendbuf,strlen(sendbuf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
	printf("send response number=%d\n",data.number);
	return 1;
}

int send_level2(struct msg_remote data)
{
	char sendbuf[512];
	sprintf(sendbuf,"2130 002\r\nlength=%d\r\nnumber=%d\r\n\r\n",data.length,commandnumber);
	write(socketfd,sendbuf,strlen(sendbuf));
	write(socketfd,data.data,data.length);
	return 1;
}

int send_level3(struct msg_remote data)
{
	int rc;
	char sendbuf[512];
	sprintf(sendbuf,"DATA 002\r\nusername=%s\r\ndate=%s\r\nlength=%d\r\nnumber=%d\r\ndatatype=plc\r\ndatalevel=1\r\n\r\n",username,data.time,data.length,data.number);
	rc=write(socketfd,sendbuf,strlen(sendbuf));
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
	rc=write(socketfd,data.data,data.length);
	if(rc<=0)
	{
		DEBUG("WRITE ERROR");
		exit(1);
	}
	printf("send a first data number=%d\n",data.number);
	datalevel1=data.number;
	pthread_mutex_lock(&mutex);
	while(!confirm)
		pthread_cond_wait(&cond,&mutex);
	confirm=0;
	pthread_mutex_unlock(&mutex);
	return 1;
}

void *remote_send()
{
	int rc;
	int init=1;
	unsigned char c;
	unsigned char real_time_data[256];
	int pipe_fd;
	int flag=0;
	printf("remote send pthread begin\n");
	int semid,shmid;
	int length;
	void*shmaddr;
	struct shm_remote sdata;
	char sendbuf[1024];
	struct msg_remote gpsdata;
	struct shm_remote *p;
	if((shmid=creatshm(2))==-1)
	{
		DEBUG("ATTACH SHM ERROR");
		exit(1);
	}
	if((shmaddr=shmat(shmid,(char*)0,0))==(char*)-1)
	{
		DEBUG("ATTACH SHM ERROR");
		exit(1);
	}
	if((semid=opensem(2))==-1)
	{
		DEBUG("OPEN ERROR");
		exit(1);
	}
	p=(struct shm_remote*)shmaddr;
	while(1)
	{
		while(real_time)
		{
		//	printf("haha\n");
			if(!init)
			{
				pipe_fd=open(FIFO_NAME,O_RDONLY);
				if(pipe_fd==-1)
				{
					DEBUG("OPEN ERROR");
					exit(1);
				}
				init=1;
			}
//			printf("azmzam\n");
		    do
			{
				rc=read(pipe_fd,&c,1);
				if(rc!=1)
				{
					DEBUG("READ ERROR");
					exit(1);
				}
//				printf("tata\n");
			}while(rc<=0);
			length=(int)c;
//			printf("length=%d\n",length);
			if(length)
			{
//				printf("xxx\n");
				rc=read(pipe_fd,real_time_data,length);
				if(rc<=0)
				{
					DEBUG("READ ERROR");
					exit(1);
				}
			}
			else
			{
				real_time=0;
				break;
			}
			sprintf(sendbuf,"2130 002\r\nlength=%d\r\nnumber=%d\r\n\r\n",length,commandnumber);
//			printf("sendbuf=%s\n",sendbuf);
			rc=write(socketfd,sendbuf,strlen(sendbuf));
			if(rc<=0)
			{
				DEBUG("WRITE ERROR");
				exit(1);
			}
			rc=write(socketfd,real_time_data,length);
			if(rc<=0)
			{
				DEBUG("WRITE ERROR");
				exit(1);
			}
		}
		if(init==1)
		{
//			printf("close pipe ok\n");
			init=0;
			close(pipe_fd);
		}

		while(!real_time)
		{
//			printf("gaga\n");
			if(msg_recv(&gpsdata))
			{
//				printf("%ld length=%d\n",gpsdata.mtype,gpsdata.length);
				switch(gpsdata.mtype)
				{
					case 1:
					send_level1(gpsdata);
					break;
					case 3:
					send_level3(gpsdata);
					break;
					default:
					break;
				}
			}
			if(real_time==1)
				break;
			if(getsem(semid))
			{
		//		printf("4\n");
				if(sem_p(semid)!=-1)
				{
			//		printf("5\n");
					if(p->written!=0)
					{
						memcpy(&sdata,p,sizeof(struct shm_remote));
						p->written=0;
						flag=1;
					//	printf("6\n");
					}
					else
						flag=0;
				}
				sem_v(semid);
				if(flag)
				{
					sprintf(sendbuf,"DATA 002\r\nusername=%s\r\ndate=%s\r\nlength=%d\r\nnumber=%d\r\ndatatype=plc\r\ndatalevel=2\r\n\r\n",username,sdata.time,sdata.length,sdata.number);
					rc=write(socketfd,sendbuf,strlen(sendbuf));
					if(rc<=0)
					{
						DEBUG("WRITE ERROR");
						exit(1);
					}
					rc=write(socketfd,sdata.data,sdata.length);
					if(rc<=0)
					{
						DEBUG("WRITE ERROR");
						exit(1);
					}
					printf("send a second data number=%d\n",sdata.number);
					datalevel2=sdata.number;
					pthread_mutex_lock(&mutex);
					while(!confirm)
						pthread_cond_wait(&cond,&mutex);
					confirm=0;
					pthread_mutex_unlock(&mutex);
				}
			}
			else
				usleep(500000);
		}
	}

//	printf("n_s=%c,value1=%s,e_w=%c,value2=%s\n",gpsdata.n_s,gpsdata.value1,gpsdata.e_w,gpsdata.value2);
	
//	sprintf(da,"%c,%s,%c,%s",gpsdata.n_s,gpsdata.value1,gpsdata.e_w,gpsdata.value2);
//	printf("da=%s\n",da);
/*	i=strlen(da);
	strcpy(data_buf,"DATA 002\r\n");
	strcat(data_buf,"datatype=gps\r\n");
	sprintf(temp,"length=%d\r\n",i);
	strcat(data_buf,temp);
	memset(temp,'\0',4096);
	gettime(time_temp);
	sprintf(time,"date=%s\r\n",time_temp);
	sprintf(name,"username=%s\r\n",username);
	strcat(data_buf,name);
	strcat(data_buf,time);
	count=INC(count);
	snprintf(temp,4096,"number=%d\r\n",count);
	strcat(data_buf,temp);
	strcat(data_buf,"\r\n");
	printf("local send:\n");
	printf("%s",data_buf);
	write(socketfd,data_buf,strlen(data_buf));
	write(socketfd,da,strlen(da));
//	printf("send data ok*************\n");
	printf("%s\n",da);
	printf("local send over!\n");
	return;
	*/
}

void gettime(char *datetime)
{
	time_t now;
	char date[100];
	struct tm *tm_now;
	time(&now);
	tm_now=localtime(&now);
	sprintf(date,"%d-%d-%d %d:%d:%d",(tm_now->tm_year+1900),(tm_now->tm_mon+1),tm_now->tm_mday,tm_now->tm_hour,tm_now->tm_min,tm_now->tm_sec);
	memcpy(datetime,date,strlen(date));
	return;
}

/*int main()
{	
    FILE *file;
	char buf[4096];
	file=fopen("config","r");
	fgets(buf,4096,file);//get the first line of config
	fclose(file);
	if(buf[5]=='0')
	{
		printf("initialization start!");
		authenticate();
		printf("initialization OK!");
	}
	login_in();
	exit(0);
}

int authenticate()
{
	
	int fd;
	int len,i=0;
	struct sockaddr_in address;
	int result;
	FILE *fp;
	char write_buf[4096],read_buf[4096],file_buf[4096],temp[4096];
	fd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(HOST_ADDRESS);
	address.sin_port = htons(13000);
	len = sizeof(address);
	result = connect(fd, (struct sockaddr *)&address, len);
	while(i<=20)
	{	
		if(result!=-1)
			break;
		result = connect(fd, (struct sockaddr *)&address, len);
		sleep(++i*5);
	}

	if(result == -1) {
		perror("oops: client1");
		exit(1);
	}
	printf("CONNECTION OK!\n");
	snprintf(write_buf,4096,"RCON");
	write(fd,write_buf,strlen(write_buf));
	read(fd,read_buf,4096);
	printf("%s\n",read_buf);
	if(strncmp(read_buf,"1010",4)!=0)//?
	{
		perror("authenticate error");
		exit(1);
	}
    printf("get answer for the RCON\n");
	write_buf="LOGI oo2\r\n";
	fp=fopen("config","r+");
	fgets(temp,4096,fp);
	strcat(write_buf,temp);
	fgets(temp,4096,fp);
	strcat(write_buf,temp);
	strcat(write_buf,"\r\n");
	printf("%s\n",write_buf);
	write(fd,write_buf,strlen(write_buf));
	read(fd,read_buf,4096);
	if(strncmp(read_buf,"1020",4)==0)
	{
		
	}

*	fgets(write_buf,4096,fp);//get the first line of config
	fgets(write_buf,4096,fp);//get the second line of config
	char *p;
	p=write_buf;
	for(;*p!='\0';p++)
	{
		if(*p==':')
		{
			for(i=0;*p!='\0';p++)
				file_buf[i++]=*(p+1);
			file_buf[i]='\0';
			break;
		}
			//strcat(file_buf,4096,write_buf+1);
			//strchr(file_buf,':') rreturn the first pointer
	}//get the user name

	snprintf(write_buf,4096,"USER %s\r\n",file_buf);
	write(fd,write_buf,strlen(write_buf));
	read(fd,read_buf,4096);
	printf("%s\n",read_buf);
	if(strncmp(read_buf,"331",3)!=0)
	{
		perror("error");
		exit(1);
	}
  //get third line of config
   fgets(write_buf,4096,fp);
   p=write_buf;
	for(;*p!='\0';p++)
	{
		if(*p==':')
		{
			for(i=0;*p!='\0';p++)
				file_buf[i++]=*(p+1);
			file_buf[i]='\0';
			break;
		}
	}
 
//send passwd	
	snprintf(write_buf,4096,"PASS %s\r\n",file_buf);
	write(fd,write_buf,strlen(write_buf));
	read(fd,read_buf,4096);
	if(strncmp(read_buf,"212",3)!=0)
	{
		perror("error");
		exit(1);
	}
	//get the fourth line of fp 
	fgets(write_buf,4096,fp);
    p=write_buf;
	for(;*p!='\0';p++)
	{
		if(*p==':')
		{
			for(i=0;*p!='\0';p++)
				file_buf[i++]=*(p+1);
			file_buf[i]='\0';
			break;
		}
	}
	printf("%s\n",file_buf);
	// receive new name and passwd
	snprintf(write_buf,4096,"CONF %s\r\n",file_buf);
	write(fd,write_buf,strlen(write_buf));
    //
	i=read(fd,read_buf,4096);
	printf("%d\n",i);
	printf("%s\n",read_buf);
	char *p1,name[4096],passwd[4096];//*name no memory
	//p1=read_buf;
    if(strncmp(read_buf,"231",3)==0)
	{
		printf("debug!1\n");
		p1=strchr(read_buf,' ');
		printf("%s\n",p1);
		for(i=0;*(++p1)!=' ';)
		{
			name[i]=*p1;
			i++;
		}
		name[i]='\0';
		printf("%s\n",name);
		for(i=0;*(++p1)!='\0';)// '/n' cause bug
		{
			passwd[i++]=*p1;
		}
		passwd[i]='\0';
	
	//sscanf(read_buf,"231 %s %s",name,passwd);
		printf("%s\n",passwd);
	}
	//write to config file
    p=name;p1=passwd;
	fseek(fp,0,SEEK_SET);//rewind(fp);
	printf("debug2!\n");
	fclose(fp);
	fp=fopen("config","w");
	fprintf(fp,"init:1\nusername:%s\npasswd:%s\nserial:0001",p,p1);
	//quit
	snprintf(write_buf,4096,"QUIT\r\n");
	write(fd,write_buf,strlen(write_buf));
	read(fd,read_buf,4096);
	if(strncmp(read_buf,"221",3)!=0)
	{
		perror("error");
		exit(1);
	}
	fclose(fp);
	close(fd);
}

int login_in()
{ 
	int ctr_fd,data_fd;
	int len,i=0;
	struct sockaddr_in address;
	int result,result1;
	FILE *fp;
	char *p;
	char write_buf[4096],read_buf[4096],file_buf[4096];
	ctr_fd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(HOST_ADDRESS);
	address.sin_port = htons(13000);
	len = sizeof(address);
	result = connect(ctr_fd, (struct sockaddr *)&address, len);
	while(i<=20)
	{

		if(result!=-1)
			break;
		result = connect(ctr_fd, (struct sockaddr *)&address, len);
		sleep(++i*5);
	}

	if(result == -1) {
		perror("oops: client1\n");
		exit(1);
	}
	printf("CONNECTION OK!\n");
	//
	snprintf(write_buf,4096,"RCON");
	write(ctr_fd,write_buf,strlen(write_buf));
	printf("%s\n",write_buf);
	read(ctr_fd,read_buf,4096);
	printf("RCON REPLY:%s\n",read_buf);
	if(strncmp(read_buf,"211",3)!=0)//?
	{
		perror("login error\n");
		exit(1);
	}
    fp=fopen("config","r+");
    fgets(write_buf,4096,fp);//get the first line of config
    fgets(write_buf,4096,fp);//get the second line of config
	p=write_buf;
	for(;*p!='\0';p++)
	{
		if(*p==':')
		{
			for(i=0;*p!='\0';p++)
				file_buf[i++]=*(p+1);
			file_buf[i]='\0';
			break;
			//strcat(file_buf,4096,write_buf+1);
			//strchr(file_buf,':') rreturn the first pointer
	}
	}
	snprintf(write_buf,4096,"USER %s\r\t",file_buf);
	write(ctr_fd,write_buf,strlen(write_buf));
	printf("send username:%s\n",write_buf);
	read(ctr_fd,read_buf,4096);
	printf("Send user reply:%s\n",read_buf);
	if(strncmp(read_buf,"331",3)!=0)
	{
		perror("error\n");
		exit(1);
	}
 
    fgets(write_buf,4096,fp);//get the third line of config
	p=write_buf;
	for(;*p!='\0';p++)
	{
		if(*p==':')
		{
			for(i=0;*p!='\0';p++)
				file_buf[i++]=*(p+1);
			file_buf[i]='\0';
			break;
		}
	}
	snprintf(write_buf,4096,"PASS %s\r\n",file_buf);
	write(ctr_fd,write_buf,strlen(write_buf));
	printf("send pass name");
	read(ctr_fd,read_buf,4096);
	printf("passwd reply:");
	if(strncmp(read_buf,"230",3)!=0)
	{
		perror("error\n");
		exit(1);
	}
	printf("login in\n");
	snprintf(write_buf,4096,"TRAN");
	write(ctr_fd,write_buf,strlen(write_buf));
	read(ctr_fd,read_buf,4096);
	printf("%s\n",read_buf);
	if(strncmp(read_buf,"125",3)!=0)
	{
		perror("error\n");
		exit(1);
	}
	printf("data tran\n");
	char newaddr[4096];
	char p0[6],p1[6];
    for(i=0;i<strlen(read_buf);i++)
	{
		read_buf[i]=read_buf[i+9];
		
	
	}
	read_buf[i]='\0';//read_buf[i-1]='\0';change \n to \0
	int j=0,m=0;
	char arr[6][4096];
	for(i=0;i<strlen(read_buf);i++)
	{
		if(read_buf[i]==',')
			{
				j++;m=0;
			}
		else
		{
			arr[j][m++]=read_buf[i];
		}
	}
	printf("%s\n",read_buf);
	for(i=0;i<3;i++)
	{
	strcat(newaddr,arr[i]);
	strcat(newaddr,".");
	}
	strcat(newaddr,arr[3]);
	printf("%s",newaddr);
		//	sscanf(read_buf,"%s,%s,%s,%s,%s,%s",arr[0],arr[1],arr[2],arr[3],arr[4],arr[5]);
	//printf("%s%s%s%s%s%s\n",arr[0],arr[1],arr[2],arr[3],arr[4],arr[5]);
	// USE DATA PORT 
	sprintf(newaddr,"%s.%s.%s.%s\n",arr[0],arr[1],arr[2],arr[3]);//?
	printf("%s\n",newaddr);
	data_fd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	int port;
	port=256*atoi(arr[4])+atoi(arr[5]);
        printf("%d\n",port);
	address.sin_addr.s_addr = inet_addr(newaddr);
	address.sin_port = htons(port);
	len = sizeof(address);
    result1=connect(data_fd, (struct sockaddr *)&address, len);
	int local_fd;
	local_fd=local_comm();
	printf("local_fd:%d\n",local_fd);
	while(i<=20)
	{
		if(result1!=-1)
			break;
		result1 = connect(data_fd, (struct sockaddr *)&address, len);
		sleep(++i*5);
	}

	if(result1 == -1) {
		perror("oops: client1\n");
		exit(1);
	}
	while(1)
	{
		read(local_fd,write_buf,21);
		sleep(5);
//	sprintf(write_buf,"voltage:%dcurrent:%d",random()%450,random()%100);
		printf("snddata:%s\n",write_buf);
		write(data_fd,write_buf,strlen(write_buf));
	}
}
int local_comm()
{

	int client_sockfd;
	int client_len;
	struct sockaddr_in client_address;
	int result,i=0;
	client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	client_address.sin_family = AF_INET;
	client_address.sin_addr.s_addr = inet_addr(PLC_ADDRESS);
	client_address.sin_port =htons(13000);
	client_len = sizeof(client_address);
	result = connect(client_sockfd, (struct sockaddr *)&client_address,client_len);
	while(i<=20)
	{	
		if(result!=-1)
			break;
		result = connect(client_sockfd, (struct sockaddr *)&client_address,client_len);
		sleep(++i*5);
	}

	if(result == -1) {
		perror("oops: client1");
		exit(1);
	}
	printf("LOCAL_COMM CONNECTION OK!\n");
	return client_sockfd;
}
*/
