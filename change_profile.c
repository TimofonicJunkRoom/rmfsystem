/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\change_profile.c
#       @date         :2015/12/02 16:20
#       @algorithm    :
==========================================================================*/
#include "change_profile.h"

#include "rmfsystem.h"

int addoraltconfig(char* conf_path,char*conf_name,char*config_buff)
{
	char config_linebuf[256];
	char line_name[40];
	char *config_sign="=";
	char *leave_line;
	int alter_sign=0;

	FILE*f;
	f=fopen(conf_path,"r+");
	if(f==NULL)
	{
		printf("OPEN CONFIG FAILED\n");
		return 0;
	}
	fseek(f,0,SEEK_END);
	long congig_lenth=ftell(f);
	int configbuf_lenth=strlen(config_buff);
	configbuf_lenth=configbuf_lenth+5;
	char sum_buf[congig_lenth+configbuf_lenth];
	memset(sum_buf,0,sizeof(sum_buf));
	fseek(f,0,SEEK_SET);
	while(fgets(config_linebuf,256,f)!=NULL)
	{
		if(strlen(config_linebuf)<3)
		{
			strcat(sum_buf,config_linebuf);
			continue;
		}
		leave_line=NULL;
		leave_line=strstr(config_linebuf,config_sign);
		if(leave_line==NULL)
		{
			strcat(sum_buf,config_linebuf);
			continue;
		}
		int leave_num=leave_line-config_linebuf;
		memset(line_name,0,sizeof(line_name));
		strncpy(line_name,config_linebuf,leave_num);
		if(strcmp(line_name,conf_name)==0)
		{
			strcat(sum_buf,config_buff);
			strcat(sum_buf,"\n");
			alter_sign=1;
		}
		else
		{
			strcat(sum_buf,config_linebuf);
		}
		if(fgetc(f)==EOF)
		{
			break;
		}
		fseek(f,-1,SEEK_CUR);
		memset(config_linebuf,0,sizeof(config_linebuf));
	}
	if(alter_sign==0)
	{
		strcat(sum_buf,config_buff);
		strcat(sum_buf,"\n");
	}
//	printf("---sum_buf--->%s<------\n",sum_buf);
	remove(conf_path);
	fclose(f);
	FILE*fp;
	fp=fopen(conf_path,"w+");
	if(fp==NULL)
	{
		printf("OPEN CONFIG FAILED\n");
		return 2;
	}
	fseek(fp,0,SEEK_SET);
	fputs(sum_buf,fp);
	fclose(fp);
}
/*
void main()
{
	addoraltconfig("./device.config","second_rate","second_rate=10");
}
*/
