/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\shm_mem.h
#       @date         :2015-12-02 10:59
#       @algorithm    :
==========================================================================*/
#ifndef _SHM_MEM_H_
#define _SHM_MEM_H_

#define SHM_SIZE (20*1024)
#define SHMID 1000
#define SEMID 2000
#define SHMID2 3000
#define SEMID2 4000

union semun
{
	int val;
	struct semid_ds*buf;
	unsigned short *array;
	struct seminfo *buf_info;
	void *pad;
};

int creatsem(int);

int opensem(int);

int sem_p(int);

int sem_v(int);

int getsem(int);

int sem_delete(int);

int wait_sem(int);

int creatshm(int);

int deleteshm(int);
#endif
