/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\mini-ntpclient.h
#       @date         :2015/12/02 10:18
#       @algorithm    :
==========================================================================*/
#ifndef _MINI_NTPCLIENT_H_
#define _MINI_NTPCLIENT_H_

#include <stdint.h>
#include <arpa/inet.h>

#define JAN_1970  0x83aa7e80      /* 2208988800 1970 - 1900 in seconds */
#define NTP_PORT  123

/* How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x) ( 4294*(x) + ( (1981*(x))>>11 ) )

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via settimeofday) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) ( ( (x) >> 12 ) - 759 * ( ( ( (x) >> 10 ) + 32768 ) >> 16 ) )

/* Converts NTP delay and dispersion, apparently in seconds scaled
 * by 65536, to microseconds.  RFC1305 states this time is in seconds,
 * doesn't mention the scaling.
 * Should somehow be the same as 1000000 * x / 65536
 */
#define sec2u(x) ( (x) * 15.2587890625 )

#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4
#define PREC -6

struct ntptime {
	unsigned int coarse;
	unsigned int fine;
};

static void send_packet(int);

static int set_time(char*,struct in_addr,uint32_t*);

static int query_server(char*);

int ntp(void);

#endif
