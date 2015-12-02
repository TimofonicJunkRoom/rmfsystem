/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\client_pipe.h
#       @date         :2015/12/02 17:51
#       @algorithm    :
==========================================================================*/

#ifndef _CLIENT_PIPE_H_
#define _CLIENT_PIPE_H_

#include "rmfsystem.h"

#define HOST_ADDRESS "192.168.0.92"

int conn(void);

int update_date_1(int);

int updaet_date_2(int);

int first_login(void);

int login(void);

void dbrecord(char*);

void dbrecord_v2(char*);

int get_line(int,char*,int);

int change_file_single_value(char*);

int recv_fork(void);

int insert_command(int,char*,char*,char*);

int change_file(char*);

int change_config(char*,char*);

int send_fork(void);

void handler(void);

void send_data(void);

void msg_init(void);

int msg_recv(struct msg_remote*);

void *remote_recv(void*);

void * remote_send(void*);

int device_change(void);

int real_time_data(void);

int recv_error(void);

void dbinit(void);

int send_signal(void);

int msg_send(struct msg_remote*);

int send_signal_usr2(void);

void write_pid_remote(void);

int data_response(void);

void main(void);

int send_level1(struct msg_remote);

int send_level2(struct msg_remote);

int send_level3(struct msg_remote);
#endif
