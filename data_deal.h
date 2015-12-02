/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\data_deal.h
#       @date         :2015/12/02 17:38
#       @algorithm    :
==========================================================================*/
#ifndef _DATA_DEAL_H_
#define _DATA_DEAL_H_

#include "rmfsystem.h"

#define INC(x) (((++x)>65536)?1:x)

void msg_init(void);

int msg_recv(int,struct msg_local*);

void dbinit(void);

int insert(struct msg_remote*);

int insert_second(int,char*,unsigned char*,int);

int msg_send(int,struct msg_remote*);

void * first_level_deal(void*);

void * second_level_deal(void*);

void * signal_wait(void*);

void write_pid_deal(void);

#endif
