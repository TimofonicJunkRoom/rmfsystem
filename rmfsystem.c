/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\rmfsystem.c
#       @date         :2015/12/04 16:11
#       @algorithm    :
==========================================================================*/
#include "rmfsystem.h"

#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sqlite3.h>
/*
 *  *state:
 *   *1:change the collecting rate
 *    *2:something happened in network
 *     *3:PLC connect error
 *      */
int send_signal(int state)
{

	int ret;
	int pid;
	char *p;
	char line[100];
	printf("state=%d\n",state);
	if(state==1)
	{
		read_file_v2("pid","local_pid",line);
		pid=atoi(line);
	}
	else if(state==2)
	{
		read_file_v2("pid","deal_pid",line);
		pid=atoi(line);
	}
	else if(state==3)
	{
		read_file_v2("pid","remote_pid",line);
		pid=atoi(line);
	}
	printf("pid=%d\n",pid);
	ret=kill((pid_t)pid,SIGUSR1);
	printf("remote:send signal SIGUSR1\n");
	if(ret==0)
		return 1;
	else 
		return 0;
}

void dbrecord_v2(char* state,sqlite3*db)
{
	int rc;
	char sql[256];
	char * errmsg=0;
	char time[100];
	gettime(time);
	sprintf(sql,"insert into device_running_statement values(\"%s\",\"%s\")",time,state);
	while(1)
	{
		rc=sqlite3_exec(db,sql,0,0,&errmsg);
		if(rc!=SQLITE_OK)
		{
			if(rc==SQLITE_BUSY)
				continue;
			DEBUG("REMOTE RECORD ERROR");
			exit(1);
		}
		else
			break;
	}
	return;
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

