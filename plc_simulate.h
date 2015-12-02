/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\plc_simulate.h
#       @date         :2015-12-02 14:40
#       @algorithm    :
==========================================================================*/
#ifndef _PLC_SIMULATE_H_
#define _PLC_SIMULATE_H_

#define LOCAL_ADDRESS "127.0.0.1"
#define UDP_PORT 2002
#define TCP_PORT 2000
#define UDP_INTERVAL 2
#define UDP_BUFF 2
#define TCP_BUFF 64*1024
#define ANGLE(a) (a+5)>360?(a=0):(a=a+5,a)
#define PI 3.141592

extern int plc_float[];

struct fetch
{
	unsigned char systemid_1;
	unsigned char systemid_2;
	unsigned char len_of_head;
	unsigned char id_op_code;
	unsigned char len_op_code;
	unsigned char op_code;
	unsigned char org_field;
	unsigned char len_org_field;
	unsigned char org_id;
	unsigned char dbnr;
	unsigned char start_address_h;
	unsigned char start_address_l;
	unsigned char len_h;
	unsigned char len_l;
	unsigned char empty_field;
	unsigned char len_empty_field;
};

struct fetch_res
{
	unsigned char systemid_1;
	unsigned char systemid_2;
	unsigned char len_of_head;
	unsigned char id_op_code;
	unsigned char len_op_code;
	unsigned char op_code;
	unsigned char ack_field;
	unsigned char len_ack_field;
	unsigned char error_field;
	unsigned char empty_field;
	unsigned char len_empty_field;
	unsigned char fill_field[5];
};

void * udp_send(void*);

void * tcp_send(void*);

int tcp_establish(void);

void response_error(int);

void response_data(struct fetch,int);

void init_struct(struct fetch_res*);

int combine(unsigned char,unsigned char);
#endif
