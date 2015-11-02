/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem/plc_simulate\plc_simulate.h
#       @date         :2015/09/14 14:45
#       @algorithm    :
==========================================================================*/

#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<netdb.h>
#include<string.h>
#include<time.h>
#include<pthread.h>
#include<math.h>
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

