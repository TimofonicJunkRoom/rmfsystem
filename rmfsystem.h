/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/linghao\rmfsystem.h
#       @date         :2015-08-25 17:33
#       @algorithm    :
==========================================================================*/
#ifndef _RMFSYSTEM_H_
#define _RMFSYSTEM_H_

#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MSGID_LOCAL 3000
#define MSGID_REMOTE 4000
#define MSG_MAX 256
#define SHM_MAX	(10*1024)
#define DB "local.db"
#define DEV_CONF "device.config"
#define PRO_CONF "process.config"
#define CONFIG "config"
#define REAL_TIME "real_time.xml"
#define FIFO_NAME "/tmp/my_fifo"

#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) printf("FILE: "__FILE__", LINE: %04d: "format"\n",__LINE__,##__VA_ARGS__)
#else
#define DEBUG(format,...)
#endif


struct msg_local
{
	long mtype;
	int length;
	unsigned char data[MSG_MAX];
};

struct shm_local
{
	int written;
	int length;
	unsigned char data[SHM_MAX];
};

struct msg_remote
{
	long mtype;
	int length;
	char command[5];
	int number;
	char time[100];
	unsigned char data[MSG_MAX];
};

struct shm_remote
{
	int written;
	int length;
	int number;
	char time[100];
	unsigned char data[SHM_MAX];
};

struct plc_struct 
{
	unsigned char org_id;
	unsigned char dbnr;
	unsigned char start_address_h;
	unsigned char start_address_l;
	unsigned char len_h;
	unsigned char len_l;
	struct plc_struct * next;
};

void gettime(char*);

#endif
