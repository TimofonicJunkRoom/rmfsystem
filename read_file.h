/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2015
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\read_file.h
#       @date         :2015/12/02 12:10
#       @algorithm    :
==========================================================================*/
#ifndef _READ_FILE_H_
#define _READ_FILE_H_
 
#define KEYVALLEN 100

char *l_trim(char*,const char*);

char *r_trim(char*,const char*);

char *a_trim(char*,const char*);

int GetProfileString(char*,char*,char*,char*);

unsigned char read_profile(char*,char*);

void read_file(char*,char*,char*);

void read_file_v1(char*,char*,char*);

#endif
