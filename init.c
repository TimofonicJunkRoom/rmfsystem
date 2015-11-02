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

#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) printf("File: "__FILE__", Line: %04d: "format"\n",__LINE__,##__VA_ARGS__)
#else 
#define DEBUG(format,...)
#endif

#define DB "local.db"

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
	
}

sqlite3* db_exist()
{
	int rc;
	sqlite3 * db;
	rc=sqlite3_open_v2(DB,&db,SQLITE_OPEN_READWRITE,NULL);
	if(rc==SQLITE_OK)
	{
		printf("database: db exist detecting ok!......\n ");
		return db;
	} 
	printf("database: db not exist!......\ncreate database!......\n");
	sqlite3_close(db);
	rc=sqlite3_open(DB,&db);
	if(rc!=SQLITE_OK)
	{
		DEBUG("create database fail!......\n");
		exit(1);
}

void db_table(sqlite3 * db)
{

}

void profile_init()
{
	profile_config();
	profile_dev();
	profile_process();
}

void profile_config()
{

}

void profile_dev()
{

}

void profile_process()
{

}

void main()
{
	db_init();
	profile_init();
}
