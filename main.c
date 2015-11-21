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
#include "init.h"

void dbinit(void);
void gettime(char*);
void dbrecord(void);
pid_t com=-1;
pid_t deal=-1;
pid_t local=-1;
sqlite3 *db;

int main()
{	
	int rc;
	int status;
	init();
	dbrecord();
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
				if((rc=execl("/home/lhw4d4/project/git/rmfsystem/comtest_pipe","comtest_pipe",NULL))<0)
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
				if((rc=execl("/home/lhw4d4/project/git/rmfsystem/data_deal","data_deal",NULL))<0)
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
				if((rc=execl("/home/lhw4d4/project/git/rmfsystem/client_pipe","client_pipe",NULL))<0)
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

void dbinit(void)
{
	db=NULL;
	char* errmsg=0;
	int rc;
	rc=sqlite3_open("local.db",&db);
	if(rc)
	{
		fprintf(stderr,"cannot open database:%s\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	else
		printf("main:db open successfully\n");
	return;
}

void dbrecord(void)
{
	dbinit();
	char time[100];
	gettime(time);
	int rc;
	char*errmsg=0;
	sqlite3_stmt *stmt=NULL;
	const char*sql="insert into device_running_statement values(?,?)";
	if(sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,0)!=SQLITE_OK)
	{
		if(stmt)
			sqlite3_finalize(stmt);
		DEBUG("PREPARE ERROR");
		exit(1);
	}
	sqlite3_bind_text(stmt,1,time,strlen(time),SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt,2,"start",5,SQLITE_TRANSIENT);
	while(1)
	{
		if(rc=sqlite3_step(stmt)!=SQLITE_DONE)
		{
			if(rc==SQLITE_BUSY)
				continue;
			DEBUG("INSERT ERROR");
			exit(1);
		}
		else
			break;
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return;	
}

void gettime(char*datetime)
{
	time_t now;
	char date[100];
	struct tm*tm_now;
	time(&now);
	tm_now=localtime(&now);
	sprintf(date,"%d-%d-%d %d:%d:%d",(tm_now->tm_year+1900),(tm_now->tm_mon+1),tm_now->tm_mday,tm_now->tm_hour,tm_now->tm_min,tm_now->tm_sec);
	memcpy(datetime,date,strlen(date));
	return;
}
