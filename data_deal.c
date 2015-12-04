/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\data_deal.c
#       @date         :2015-12-02 17:38
#       @algorithm    :
==========================================================================*/
#include "data_deal.h"

#include <time.h>
#include <sys/time.h>
#include <sqlite3.h>
#include "shm_mem.h"
#include <pthread.h>
#include <signal.h>
#include "read_file.h"
#include <sys/shm.h>
#include <sys/types.h>

int real_time=0;
int networkflag=0;
int count1=0;
int count2=0;
pthread_mutex_t mut;

int msg_local;
int msg_remote;
sqlite3 *db;

void write_pid_deal()
{
	char line[50];
	char newline[50];
	int pid;
	FILE*fp;
	FILE*fpnew;
	fp=fopen(PRO_CONF,"r+");
	if(fp==NULL)
		exit(1);
	fpnew=fopen("process.config.new","w");
	if(fpnew==NULL)
	{
		DEBUG("OPEN FILE ERROR");
		exit(1);
	}
	while(fgets(line,50,fp))
	{
		if(strncmp(line,"deal_pid=%d\n",8)==0)
		{
			pid=getpid();
			sprintf(newline,"deal_pid=%d\n",pid);
			fputs(newline,fpnew);
		}
		else
			fputs(line,fpnew);
	}
	fclose(fp);
	fclose(fpnew);
	remove(PRO_CONF);
	rename("process.config.new",PRO_CONF);
//	printf("	write process ok\n");
	return;
}

void *signal_wait(void *arg)
{
	int result;
	char value[20];
	printf("deal:signal pthread begin\n");
	int err;
	int signo;
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset,SIGUSR2);
	sigaddset(&sigset,SIGUSR1);
	while(1)
	{
		err=sigwait(&sigset,&signo);
		if(err!=0)
		{
			DEBUG("SIGNAL ERROR:%d",err);
			exit(1);
		}
		switch(signo)
		{
			case SIGUSR2:
				if(real_time)
					real_time=0;
				else
					real_time=1;
				break;
			case SIGUSR1:
				printf("1\n");
				read_file("setting","setting",value);
				result=atoi(value);
				if(result!=2)
					continue;
				printf("deal:network\n");
				if(result==2)
				{
					if(networkflag)
						networkflag=0;
					else
						networkflag=1;
				}
				break;
		}
		
//		printf("real_time=%d\n",real_time);
	}
}

void gettime(char*datetime)
{
	time_t now;
	char date[100];
	struct tm *tm_now;
	time(&now);
	tm_now=localtime(&now);
	sprintf(date,"%d-%d-%d %d:%d:%d",(tm_now->tm_year+1900),(tm_now->tm_mon+1),tm_now->tm_mday,tm_now->tm_hour,tm_now->tm_min,tm_now->tm_sec);
	strcpy(datetime,date);
	return;
}

void dbinit(void)
{
	db=NULL;
	char *errmsg=0;
	int rc;
	rc=sqlite3_open("local.db",&db);
	if(rc)
	{
		fprintf(stderr,"cannot open database:%s\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		pthread_exit((void*)1);
	}
	else
		printf("deal:deal open local.db successfully\n");
	return;
}


int msg_recv(int msgid,struct msg_local*data)
{
	int ret;
	int length;
	length=sizeof(struct msg_local)-sizeof(long);
//	printf("lxxength=%d\n",length);
//	printf("1\n");
//	printf("waiting for recv\n");
	if((ret=msgrcv(msgid,(void*)data,length,0,0))==-1)
	{
		DEBUG("MSGRCV ERROR");
		exit(1);
	}
//	printf("	msgrcv ok\n");
	return 0;
}

/*msg=1  remote
 *msg=0  local
 */
void msg_init(void)
{
	int msgid;
	msgid=msgget((key_t)MSGID_REMOTE,0666|IPC_CREAT);
	if(msgid==-1)
	{
		DEBUG("DEAL MSG REMOTE ERROR");
		exit(-1);
	}
	msg_remote=msgid;
	msgid=msgget((key_t)MSGID_LOCAL,0666|IPC_CREAT);
	if(msgid==-1)
	{
		DEBUG("DEAL MSG LOCAL ERROR");
		exit(-1);
	}
	msg_local=msgid;
	printf("deal:deal msg init ok\n");
	return;
}

int insert(struct msg_remote *data)
{
	int rc;
	const char *sql="insert into device_first_level_data values(?,?,0,?)";
	sqlite3_stmt *stmt;
	char*errmsg=0;
//	printf("time=%s,a=%c,value1=%s,b=%c,value2=%s\n",time,a,value1,b,value2);
//	sprintf(sql,"insert into device_first_level_data values(\"%d\",\"%s\",0,\"%s\")",data->number,data->time,data->data);
//	printf("sql=%s\n",sql);
//	pthread_mutex_lock(&mut);
	if(sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,0)!=SQLITE_OK)
	{
		if(stmt)
			sqlite3_finalize(stmt);
		DEBUG("PREPARE ERROR");
		exit(1);
	}
	sqlite3_bind_int(stmt,1,data->number);
	sqlite3_bind_text(stmt,2,data->time,strlen(data->time),SQLITE_TRANSIENT);
	sqlite3_bind_blob(stmt,3,data->data,data->length,NULL);
	while(1)
	{
		rc=sqlite3_step(stmt);
		if(rc!=SQLITE_DONE)
		{
			if(rc==SQLITE_BUSY)
				continue;
			DEBUG("DEAL SLQITE3 ERROR");
			printf("%s\n",errmsg);
			exit(1);
		}
		else
			break;
	}
	sqlite3_finalize(stmt);
//	pthread_mutex_unlock(&mut);
//		printf("insert success\n");
	return 1;
}

int insert_second(int number,char*time,unsigned char *data,int length)
{
	int rc;
	char*errmsg=0;
	sqlite3_stmt *stmt=NULL;
	const char*sql="insert into device_second_level_data values(?,?,0,?)";
//	pthread_mutex_lock(&mut);
//	printf("1\n");
//
	if(sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,0)!=SQLITE_OK)
	{
		if(stmt)
			sqlite3_finalize(stmt);
		DEBUG("PREPARE ERROR");
		exit(1);
	}
	sqlite3_bind_int(stmt,1,number);
	sqlite3_bind_text(stmt,2,time,strlen(time),SQLITE_TRANSIENT);
	sqlite3_bind_blob(stmt,3,data,length,NULL);
	while(1)
	{
		if((rc=sqlite3_step(stmt))!=SQLITE_DONE)
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
//	printf("1\n");
//	pthread_mutex_unlock(&mut);
//	printf("1\n");
	return 1;
}

int msg_send(int msg,struct msg_remote*data)
{
	int length;
//	printf("length=%d\n",data->length);
	length=sizeof(struct msg_remote)-sizeof(long);
	if(msgsnd(msg,(void*)data,length,0)==-1)
	{
		DEBUG("DEAL MSGSEND ERROR");
		exit(1);
	}
	else
//		printf("msgsnd ok\n");
	return 0;
}
void* first_level_deal(void *arg)
{
	struct msg_local data;
	struct msg_remote data1;
	char time[100];
	int i;
	printf("deal:first level begin\n");
	msg_init();
	while(1)
	{
//		printf("msg_local=%d\n",msg_local);
		msg_recv(msg_local,&data);
		gettime(time);
		INC(count1);
		data1.mtype=3;
		data1.number=count1;
		data1.length=data.length;
		strcpy(data1.time,time);
//		printf("lengtmmxxh=%d\n",data.length);
		memcpy(data1.data,data.data,data.length);
		insert(&data1);
//		printf("1isnert\n");
		if(!real_time&&!networkflag)
		{
			msg_send(msg_remote,&data1);
//			printf("send msg ok ag\n");
		}
	}
	exit(1);
}

void*second_level_deal(void *arg)
{
	int length;
	int rc,flag=0;
	unsigned char buff[SHM_MAX];
	printf("deal:second level begin\n");
	struct shm_remote *temp;
	char time[100];
	struct shm_local *shm_data;
	int semid,shmid,semid2,shmid2;
	void *shmaddr,*shmaddr2;
	if((shmid=creatshm(1))==-1)
	{
		DEBUG("ATTACH SHM ERROR");
		exit(1);
	}
	if((shmaddr=shmat(shmid,(char*)0,0))==(char*)-1)
	{
		DEBUG("ATTACH SHM ERROR");
		exit(1);
	}
	if((semid=opensem(1))==-1)
	{
		DEBUG("OPEN SEM ERROR");
		exit(1);
	}
	if((shmid2=creatshm(2))==-1)
	{
		DEBUG("ATTACH SHM ERROR");
		exit(1);
	}
	if((shmaddr2=shmat(shmid2,(char*)0,0))==(char*)-1)
	{
		DEBUG("ATTACH SHM ERROR");
		exit(1);
	}
	if((semid2=creatsem(2))==-1)
	{
		DEBUG("ATTACH CREATE SEM ERROR");
		exit(1);
	}
	shm_data=(struct shm_local*)shmaddr;
	temp=(struct shm_remote*)shmaddr2;
	temp->written=0;
	while(1)
	{
//		printf("networdflag=%d\n",networkflag);
		if(sem_p(semid)==-1)
			break;
		if(shm_data->written!=0)
		{
//			printf("xx\n");
			length=shm_data->length;
			memcpy(buff,shm_data->data,length);
			shm_data->written=0;
			flag=1;
		}
		else
			flag=0;
		sem_v(semid);
		if(flag)
		{
//			printf("2insert\n");
			gettime(time);
			INC(count2);
			insert_second(count2,time,buff,length);
		}
		else
			usleep(100000);
		while(flag&&!networkflag)
		{
	//		printf("ma\n");
			if(sem_p(semid2)==-1)
				exit(1);
			if(temp->written==0)
			{
//				printf("sb\n");
				strcpy(temp->time,time);
				temp->number=count2;
				temp->length=length;
				memcpy(temp->data,buff,SHM_MAX);
				temp->written=1;
				sem_v(semid2);
				break;
			}
			sem_v(semid2);
			usleep(100000);
		}
	}
	exit(1);
}

int main()
{
	int err;
	sigset_t set;
	pthread_t tid1;
	pthread_t tid2;
	pthread_t tid3;
	pthread_attr_t attr;
	dbinit();
	write_pid_deal();
	sigemptyset(&set);
//	printf("1\n");
	sigaddset(&set,SIGUSR2);
	sigaddset(&set,SIGUSR1);
	err=pthread_sigmask(SIG_BLOCK,&set,NULL);
	if(err!=0)
	{
		DEBUG("ERROR");
		exit(1);
	}
	pthread_mutex_init(&mut,NULL);
	err=pthread_attr_init(&attr);
	if(err!=0)
	{
		DEBUG("ERROR");
		exit(1);
	}
	err=pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if(err==0)
	{
		err=pthread_create(&tid1,&attr,first_level_deal,NULL);
		if(err!=0)
			exit(1);
		err=pthread_create(&tid2,&attr,second_level_deal,NULL);
		if(err!=0)
			exit(1);
		err=pthread_create(&tid3,&attr,signal_wait,NULL);
		if(err!=0)
			exit(1);
		pthread_attr_destroy(&attr);
	}
	pthread_exit((void*)1);
}
