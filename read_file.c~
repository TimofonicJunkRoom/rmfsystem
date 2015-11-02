/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\read_file.c
#       @date         :2015/09/23 20:11
#       @algorithm    :
==========================================================================*/
#include "rmfsystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
 
#define KEYVALLEN 100

struct plc_struct *plc_head=NULL;

/*   É¾³ý×ó±ßµÄ¿Õ¸ñ   */
char * l_trim(char * szOutput, const char *szInput)
{
 assert(szInput != NULL);
 assert(szOutput != NULL);
 assert(szOutput != szInput);
 for(NULL; *szInput != '\0' && isspace(*szInput); ++szInput){
  ;
 }
 return strcpy(szOutput, szInput);
}
 
/*   É¾³ýÓÒ±ßµÄ¿Õ¸ñ   */
char *r_trim(char *szOutput, const char *szInput)
{
 char *p = NULL;
 assert(szInput != NULL);
 assert(szOutput != NULL);
 assert(szOutput != szInput);
 strcpy(szOutput, szInput);
 for(p = szOutput + strlen(szOutput) - 1; p >= szOutput && isspace(*p); --p){
  ;
 }
 *(++p) = '\0';
 return szOutput;
}
 
/*   É¾³ýÁ½±ßµÄ¿Õ¸ñ   */
char * a_trim(char * szOutput, const char * szInput)
{
 char *p = NULL;
 assert(szInput != NULL);
 assert(szOutput != NULL);
 l_trim(szOutput, szInput);
 for(p = szOutput + strlen(szOutput) - 1;p >= szOutput && isspace(*p); --p){
  ;
 }
 *(++p) = '\0';
 return szOutput;
}
 
 
int GetProfileString(char *profile, char *AppName, char *KeyName, char *KeyVal )
{
 char appname[32],keyname[32];
 char *buf,*c;
 char buf_i[KEYVALLEN], buf_o[KEYVALLEN];
 FILE *fp;
 int found=0; /* 1 AppName 2 KeyName */
 if( (fp=fopen( profile,"r" ))==NULL ){
  printf( "openfile [%s] error [%s]\n",profile,strerror(errno) );
  return(-1);
 }
 fseek( fp, 0, SEEK_SET );
 memset( appname, 0, sizeof(appname) );
 sprintf( appname,"[%s]", AppName );
 
 while( !feof(fp) && fgets( buf_i, KEYVALLEN, fp )!=NULL ){
  l_trim(buf_o, buf_i);
  if( strlen(buf_o) <= 0 )
   continue;
  buf = NULL;
  buf = buf_o;
 
  if( found == 0 ){
   if( buf[0] != '[' ) {
    continue;
   } else if ( strncmp(buf,appname,strlen(appname))==0 ){
    found = 1;
    continue;
   }
 
  } else if( found == 1 ){
   if( buf[0] == '#' ){
    continue;
   } else if ( buf[0] == '[' ) {
    break;
   } else {
    if( (c = (char*)strchr(buf, '=')) == NULL )
     continue;
    memset( keyname, 0, sizeof(keyname) );
 
   sscanf( buf, "%[^=|^ |^\t]", keyname );
    if( strcmp(keyname, KeyName) == 0 ){
     sscanf( ++c, "%[^\n]", KeyVal );
     char *KeyVal_o = (char *)malloc(strlen(KeyVal) + 1);
     if(KeyVal_o != NULL){
      memset(KeyVal_o, 0, sizeof(KeyVal_o));
      a_trim(KeyVal_o, KeyVal);
      if(KeyVal_o && strlen(KeyVal_o) > 0)
       strcpy(KeyVal, KeyVal_o);
      free(KeyVal_o);
      KeyVal_o = NULL;
     }
     found = 2;
     break;
    } else {
     continue;
    }
   }
  }
 }
 fclose( fp );
 if( found == 2 )
  return(0);
 else
  return(-1);
}
// an example
unsigned char read_file(char *part1,char* part2)
{
		char para[20];
        unsigned char m;
        GetProfileString("./device.config", part1, part2,para);
		m=(unsigned char)strtol(para,NULL,16);
		return m;
}

void main()
{
	int i;
	struct plc_struct *q=NULL;
	int number;
	char part1[32];
	char part2[32];
	unsigned char org_id;
	unsigned char dbnr;
	unsigned char start_address_h;
	unsigned char start_address_l;
	unsigned char len_h;
	unsigned char len_l;
	struct plc_struct *p;
	p=plc_head;
	strcpy(part1,"collect_area");
	strcpy(part2,"org_id");
	org_id=read_file(part1,part2);
//	printf("%2x\n",org_id);
	strcpy(part1,"number_area");
	strcpy(part2,"number");
	number=(int)read_file(part1,part2);
//	printf("%d\n",number);
//	printf("number=%d org_id=%2x\n",number,org_id);
	for(i=1;i<=number;i++)
	{
		struct plc_struct* newnode;
		newnode=(struct plc_struct*)malloc(sizeof(struct plc_struct));
		newnode->org_id=org_id;
		sprintf(part1,"%d",i);
		strcpy(part2,"area");
		dbnr=read_file(part1,part2);
	//	printf("1\n");
		newnode->dbnr=dbnr;
		strcpy(part2,"start_address_h");
		start_address_h=read_file(part1,part2);
		newnode->start_address_h=start_address_h;
		strcpy(part2,"start_address_l");
		start_address_l=read_file(part1,part2);
		newnode->start_address_l=start_address_l;
		strcpy(part2,"len_h");
		len_h=read_file(part1,part2);
		newnode->len_h=len_h;
		strcpy(part2,"len_l");
		len_l=read_file(part1,part2);
		newnode->len_l=len_l;
	//	printf("1\n");
		if(plc_head==NULL)
		{
			p=newnode;
			plc_head=p;
			p->next=NULL;
		//	printf("%02x\n",plc_head->dbnr);
		}
		else
		{
			p->next=newnode;
			p=p->next;
			p->next=NULL;
		//	printf("%02x\n",p->dbnr);
		}
		//printf("1\n");
	}
//	printf("%02x\n",plc_head->dbnr);
	q=plc_head;
	for(;q!=NULL;q=q->next)
	{
		printf("2\n");
		printf("%2x\n",q->org_id);
		fflush(stdout);
		printf("%02x\n",q->dbnr);
	}
}
