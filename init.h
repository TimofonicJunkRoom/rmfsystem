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
#include "NTP.h"
#include "read_file.h"
#include "change_profile.h"

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
	time_init();
	db_init();
	profile_init();
}
