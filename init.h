/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\init.c
#       @date         :2015/11/02 09:38
#       @algorithm    :
==========================================================================*/

#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/timex.h>
#include "read_file.h"
//#include "NTP.h"
//#include "read_file.h"
//#include "change_profile.h"
#include"scan.h"
//#include "ntp.h"
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

#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) printf("File: "__FILE__", Line: %04d: "format"\n",__LINE__,##__VA_ARGS__)
#else 
#define DEBUG(format,...)
#endif

#define DB "local.db"
#define CONFIG "config"
#define DEV_CONFIG "device.config"
#define PRO_CONFIG "process.config"

void db_init(void);
sqlite3* db_exist(void);
void db_table(sqlite3 *db);
void profile_init(void);
void profile_config(void);
void profile_dev(void);
void profile_process(void);

void db_init()
{
	sqlite3 * db;
	printf("database: sqlite3 detecting!......\n");
	db=db_exist();
	db_table(db);
	printf("database ok!......\n");
	return;
}

sqlite3* db_exist()
{
	int rc;
	sqlite3 * db;
	rc=sqlite3_open_v2(DB,&db,SQLITE_OPEN_READWRITE,NULL);
	if(rc==SQLITE_OK)
	{
		printf("database: db exist detecting ok!......\n");
		return db;
	} 
	printf("database: db not exist!......\ncreate database!......\n");
	sqlite3_close(db);
	while(1)
	{
		rc=sqlite3_open(DB,&db);
		if(rc!=SQLITE_OK)
		{
			DEBUG("create database fail!......\n");
			sleep(3);
		}
		else
		{
			printf("create database successfully!......\n");
			return db;
		}
	}
}

void db_table(sqlite3 * db)
{
//	printf("1\n");
	char **result;
	int nrow,ncolumn;
	int rc;
	char * errmsg;
	char * sql="SELECT count(*) FROM device_first_level_data";
	char * sql1="SELECT count(*) FROM device_second_level_data";
	char * sql2="SELECT count(*) FROM device_command";
	rc=sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg);
//	printf("1\n");
	if(rc!=SQLITE_OK)
	{
		DEBUG("database: device_first_level_data fail!......\n%s",errmsg);
		char *sql_table="create table device_first_level_data(number integer,time text,confirm integer,data blob)";
		rc=sqlite3_exec(db,sql_table,NULL,NULL,&errmsg);
		if(rc!=SQLITE_OK)
		{
			DEBUG("create table fail!......\n%s",errmsg);
			sqlite3_close(db);
		}
	}
	printf("database: table device_first_level_data detected......\n");
	rc=sqlite3_get_table(db,sql1,&result,&nrow,&ncolumn,&errmsg);
	if(rc!=SQLITE_OK)
	{
		DEBUG("database: device_second_level_data fail!......\n%s",errmsg);
		char *sql_table1="create table device_second_level_data(number integer,time text,confirm integer,data blob)";
		rc=sqlite3_exec(db,sql_table1,NULL,NULL,&errmsg);
		if(rc!=SQLITE_OK)
		{
			DEBUG("create table fail!......\n%s",errmsg);
			sqlite3_close(db);
		}
	}
	printf("database: table device_second_level_data detected......\n");
	rc=sqlite3_get_table(db,sql2,&result,&nrow,&ncolumn,&errmsg);
	if(rc!=SQLITE_OK)
	{
		DEBUG("database: device_command fail!......\n%s",errmsg);
		char *sql_table2="create table device_command(number integer,time text,command text,context text,reuslt text)";
		rc=sqlite3_exec(db,sql_table2,NULL,NULL,&errmsg);
		if(rc!=SQLITE_OK)
		{
			DEBUG("create table fail!......\n%s",errmsg);
			sqlite3_close(db);
		}
	}
	printf("database: table device_command detected......\n");
}

void profile_init()
{
	profile_config();
	profile_dev();
	profile_process();
}

void profile_config()
{
	int rc;
	rc=access(CONFIG,0);
	if(rc!=0)
	{
		DEBUG("profile: %s not exist!......\n",CONFIG);
		exit(1);
	}
	printf("profile: %s detect ok!......\n",CONFIG);
}

void profile_dev()
{
	int rc;
	char value[5];
	rc=access(DEV_CONFIG,0);
	if(rc!=0)
	{
		DEBUG("profile: %s not exist!......\n",DEV_CONFIG);
		exit(1);
	}
	printf("profile: %s detect ok!......\n",DEV_CONFIG);
	read_file("real_time","real_time_data",value);
	rc=atoi(value);
	if(rc==1)
		addoraltconfig(DEV_CONFIG,"real_time_data","real_time_data=0");
}

void profile_process()
{
	int rc;
	char value[5];
	rc=access(PRO_CONFIG,0);
	if(rc!=0)
	{
		DEBUG("profile: %s not exist!......\n",PRO_CONFIG);
		exit(1);
	}
	printf("profile: %s detect ok!......\n",PRO_CONFIG);

}

void init()
{
//	time_init();
	ntp();
	db_init();
	profile_init();
}


struct ntptime {
	unsigned int coarse;
	unsigned int fine;
};


static void send_packet(int sd)
{
	uint32_t data[12];
	struct timeval now;

	memset(data, 0, sizeof(data));
	data[0] = htonl (
		( LI << 30 ) | ( VN << 27 ) | ( MODE << 24 ) |
		( STRATUM << 16) | ( POLL << 8 ) | ( PREC & 0xff ) );
	data[1] = htonl(1<<16);  /* Root Delay (seconds) */
	data[2] = htonl(1<<16);  /* Root Dispersion (seconds) */
	gettimeofday(&now,NULL);
	data[10] = htonl(now.tv_sec + JAN_1970); /* Transmit Timestamp coarse */
	data[11] = htonl(NTPFRAC(now.tv_usec));  /* Transmit Timestamp fine   */
	send(sd,data,48,0);
}


static int set_time(char *srv, struct in_addr addr, uint32_t *data)
{
	struct timeval tv;

	tv.tv_sec  = ntohl(((uint32_t *)data)[10]) - JAN_1970;
	tv.tv_usec = USEC(ntohl(((uint32_t *)data)[11]));
	if (settimeofday(&tv, NULL) < 0) {
		perror("settimeofday");
		return 1;	/* Ouch, this should not happen :-( */
	}

	syslog(LOG_DAEMON | LOG_INFO, "Time set from %s [%s].", srv, inet_ntoa(addr));

	return 0;		/* All good, time set! */
}

static int query_server(char *srv)
{
	int sd, rc;
	struct pollfd pfd;
	struct hostent *he;
	struct sockaddr_in sa;

	sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sd == -1) {
		perror("socket error");
		return -1;	/* Fatal error, cannot even create a socket? */
	}

//	he = gethostbyaddr(srv,4,AF_INET);
	he = gethostbyname(srv);
	if (!he) {
		perror("gethostbyname");
		close(sd);

		return 1;	/* Failure in name resolution. */
	}

	memset(&sa, 0, sizeof(sa));
	memcpy(&sa.sin_addr, he->h_addr_list[0], sizeof(sa.sin_addr));
	sa.sin_port = htons(NTP_PORT);
	sa.sin_family = AF_INET;

	syslog(LOG_DAEMON | LOG_DEBUG, "Connecting to %s [%s] ...", srv, inet_ntoa(sa.sin_addr));
	if (connect(sd, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
		perror("connect");
		close(sd);

		return 1;      /* Cannot connect to server, try next. */
	}

	/* Send NTP query to server ... */
	send_packet(sd);

	/* Wait for reply from server ... */
	pfd.fd = sd;
	pfd.events = POLLIN;
	rc = poll(&pfd, 1, 10000);
	if (rc == 1) {
		int len;
		uint32_t packet[12];

		//syslog(LOG_DAEMON | LOG_DEBUG, "Received packet from server ...");
		len = recv(sd, packet, sizeof(packet), 0);
		if (len == sizeof(packet)) {
			close(sd);

			/* Looks good, try setting time on host ... */
			if (set_time(srv, sa.sin_addr, packet))
				return -1; /* Fatal error */

			return 0;          /* All done! :) */
		}
	} else if (rc == 0) {
		syslog(LOG_DAEMON | LOG_DEBUG, "Timed out waiting for %s [%s].",
		       srv, inet_ntoa(sa.sin_addr));
	}

	close(sd);

	return 1;			   /* No luck, try next server. */
}

/* Connects to each server listed on the command line and sets the time */
int ntp()
{
	int i;
	char value[30];
	memset(value,'\0',sizeof(value));
	read_file("NTP","address",value);
	if(strlen(value)!=0) {
		int rc = query_server(value);

		/* Done, time set! */
		if (0 == rc){
			printf("set time ok!\n");
			system("date");
			return 0;
		}
		/* Fatal error, exit now. */
		if (-1 == rc)
			return 1;
	}
	return 1;
}

