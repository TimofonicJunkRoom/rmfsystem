/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\doublesem.c
#       @date         :2015/11/15 09:40
#       @algorithm    :
==========================================================================*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/sem.h>

#define PUT 5000
#define GET 6000

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

int sem_get();
int init_sem(int,int);
int del_sem(int);
int sem_p(int);
int sem_v(int);

int sem_get(int name)
{
	int sem_id;
	if((sem_id=semget((key_t)name,1,IPC_CREAT|0666))==-1)
	{
		perror("get error");
		return -1;
	}
	return sem_id;
}

int init_sem(int semid,int init)
{
	union semun sem_union;
	sem_union.val=init;
	if(semctl(semid,0,SETVAL,sem_union)==-1)
	{
		perror("init error");
		return -1;
	}
	return 0;
}

int del_sem(int sem_id)
{
	union semun sem_union;
	if (semctl(sem_id,0,IPC_RMID,sem_union)==-1)
	{
		perror("delete error");
		return -1;
	}
}

int sem_p(int sem_id)
{
	struct sembuf sem_b;
	sem_b.sem_num=0;
	sem_b.sem_op=-1;
	sem_b.sem_flg=SEM_UNDO;

	if(semop(sem_id,&sem_b,1)==-1)
	{
		perror("p operation");
		return -1;
	}
	return 0;
}

int sem_v(int sem_id)
{
	struct sembuf sem_b;
	sem_b.sem_num=0;
	sem_b.sem_op=1;
	sem_b.sem_flg=SEM_UNDO;

	if(semop(sem_id,&sem_b,1)==-1)
	{
		perror("v operation");
		return -1;
	}
	return 0;
}

int main()
{
	pid_t result;
	int sem_id1,sem_id2;
	sem_id1=sem_get(PUT);
	init_sem(sem_id1,1);
	sem_id2=sem_get(GET);
	init_sem(sem_id2,0);
	result=fork();
	if(result==-1)
	{
		perror("fork");
	}
	else if(result==0)
	{
		while(1)
		{
			sem_p(sem_id1);
			printf("put something\n");
			sem_v(sem_id2);
			sleep(2);
		}
	}
	result=fork();
	if(result==-1)
	{
		perror("fork");
	}
	else if(result==0)
	{
		while(1)
		{
			sem_p(sem_id2);
			printf("get something\n");
			sem_v(sem_id1);
		}
	}
	while(wait()!=-1)
		;
	return 0;
}
