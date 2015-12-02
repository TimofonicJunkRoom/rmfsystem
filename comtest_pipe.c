/*==========================================================================
#       COPYRIGHT NOTICE
#       Copyright (c) 2014
#       All rights reserved
#
#       @author       :Ling hao
#       @qq           :119642282@qq.com
#       @file         :/home/lhw4d4/project/git/rmfsystem\comtest_pipe.c
#       @date         :2015-10-07 16:26
#       @algorithm    :
==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "rmfsystem.h"
#include "shm_mem.h"
#include <signal.h>
#include <pthread.h>
#include "read_file.h"
#include "change_profile.h"
#include "plc_simulate.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <limits.h>


#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) printf("FIle: "__FILE__", Line: %04d : "format"\n",__LINE__,##__VA_ARGS__)
#else
#define DEBUG(format,...)
#endif
#define PLC_ADDR "192.168.0.1"
#define TCP_PORT 2000
#define UDP_PORT 2002
#define PLC_LEN (2*1024)
#define REQUEST 16
#define REAL_TIME_NUM 20
#define REAL_TIME "real_time.xml"
#define FIFO_NAME "/tmp/my_fifo"
#define BUFF_SIZE PIPE_BUF
int real_time_data=0;
int second_rate=1;
int remote_msgid;
int local_msgid;
struct plc_struct * plc_head=NULL;

struct plc_loc
{
	int used;
	int offset;
	int location;
	int length;
};

struct plc_loc real_time_loc[REAL_TIME_NUM];

int tcp_connect();

void msg_init();
void *first_level_recv();
void *second_level_recv();
void *signal_wait();
void second_rate_change();
void real_time_deal();
void write_pid_local();
void xml_read(int);

void write_pid_local()
{
	char line[50];
	char newline[50];
	int pid;
	FILE*fp;
	FILE*fpnew;
	fp=fopen(PRO_CONF,"r+");
	if(fp==NULL)
	{
		printf("open porcess_config fail\n");
		exit(1);
	}
	fpnew=fopen("process.config.new","w");
	if(fpnew==NULL)
	{
		printf("open process_config new fial\n");
		exit(1);
	}
	while(fgets(line,50,fp))
	{
		if(strncmp(line,"local_pid",9)==0)
		{
			pid=getpid();
			sprintf(newline,"local_pid=%d\n",pid);
			fputs(newline,fpnew);
		}
		else
			fputs(line,fpnew);
	}
	fclose(fp);
	fclose(fpnew);
	remove(PRO_CONF);
	rename("process.config.new",PRO_CONF);
	printf("local:write process_config ok\n");
	return;
}



int tcp_connect()
{
	int client_sockfd;
	int client_len;
	struct sockaddr_in client_address;
	int result,i=0;
	client_sockfd=socket(AF_INET,SOCK_STREAM,0);
	client_address.sin_family=AF_INET;
	client_address.sin_addr.s_addr=inet_addr(PLC_ADDR);
	client_address.sin_port=htons(TCP_PORT);
	client_len=sizeof(client_address);
	do
	{
		result=connect(client_sockfd,(struct sockaddr*)&client_address,client_len);
		if(result!=-1)
			break;
		sleep(++i*3);
	}while(i<20);
//	printf("1\n");
	if(result==-1)
	{
		DEBUG("connect error");
		exit(1);
	}
	return client_sockfd;
}
/*
void real_time_deal()
{
	char buff[50];
	FILE*fp;
	char *p;
	
	fp=fopen(DEV_CONF,"r");
	if(fp==NULL)
	{
		printf("open device.config");
		return;
	}
	//printf("1\n");
	while((fgets(buff,50,fp))!=NULL)
	{
		if(strncmp(buff,"real_time_data",14)==0)
		{
			p=strchr(buff,'=');
			p++;
			buff[strlen(buff)-1]='\0';
			real_time_data=atoi(p);
			printf("real_time_data=%d\n",real_time_data);
		}
	}
	fclose(fp);
	return;
}


void second_rate_change()
{	
	char buff[50];
	FILE*fp;
	char *p;
	fp=fopen(DEV_CONF,"r");
	if(fp==NULL)
	{
		printf("open device.config");
		return;
	}
	while((fgets(buff,50,fp))!=NULL)
	{
		if(strncmp(buff,"second_rate",11)==0)
		{
			p=strchr(buff,'=');
			p++;
			buff[strlen(buff)-1]='\0';
			second_rate=atoi(p);
			printf("second_rate=%d\n",second_rate);
		}
	}
	fclose(fp);
	return;
}
*/
void *signal_wait()
{
	int result;
	int err;
	int rate;
	int signo;
	char value[20];
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset,SIGUSR1);
	sigaddset(&sigset,SIGUSR2);
	while(1)
	{
		err=sigwait(&sigset,&signo);
		if(err!=0)
		{
			DEBUG("SIG ERROR");
			exit(1);
		}
		switch(signo)
		{
			case SIGUSR1:
				read_file("setting","setting",value);
				result=atoi(value);
				if(result==1)
				{
					read_file("collect_rate","second_rate",value);
					second_rate=atoi(value);
					printf("local:second rate=%d\n",second_rate);
				}
				break;
			case SIGUSR2:
	//			printf("xmldd\n");
				read_file("real_time","real_time_data",value);
				real_time_data=atoi(value);
				if(real_time_data!=0)
				{
					printf("local:read_xml\n");
					xml_read(real_time_data);
					real_time_data=1;
				}
				break;
			case SIGINT:
				break;
			default:
				exit(1);
		}
	}
}

void xml_read(int partid)
{
	char xml[20];
	int i=0;
	int n;
	xmlChar* value=NULL;
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr sensorcur;
	xmlNodePtr detail;
	xmlKeepBlanksDefault(0); //take care!
	doc=xmlParseFile(REAL_TIME);
	sprintf(xml,"section%d",partid);
	while(i<REAL_TIME_NUM)
	{
		real_time_loc[i].used=0;
		i++;
	}
	if(doc==NULL)
	{
		fprintf(stderr,"doc not parse\n");
		return;
	}
	cur=xmlDocGetRootElement(doc);
	if(cur==NULL)
	{
		fprintf(stderr,"empty doc\n");
		return;
	}
	if(xmlStrcmp(cur->name,(const xmlChar*)"root"))
	{
		fprintf(stderr,"wrong type\n");
		xmlFreeDoc(doc);
		return;
	}
	cur=cur->xmlChildrenNode;
//	printf("1\n");
	while(cur!=NULL)
	{

		if(!xmlStrcmp(cur->name,(const xmlChar*)xml))
		{
		//	printf("1\n");
			sensorcur=cur->xmlChildrenNode;
			i=0;
			while(sensorcur!=NULL)
			{
				real_time_loc[i].used=1;
				if(!xmlStrcmp(sensorcur->name,(const xmlChar*)"sensor"))
				{
			//		printf("google\n");
					value=xmlGetProp(sensorcur,"valueType");
					if(!strcmp(value,"BOOL"))
					{
						value=xmlGetProp(sensorcur,"dataLoc");
						real_time_loc[i].location=atoi(value);
						value=xmlGetProp(sensorcur,"offset");
						real_time_loc[i].offset=atoi(value);
						real_time_loc[i].length=0;
					}
					else if(!strcmp(value,"INT"))
					{
						value=xmlGetProp(sensorcur,"dataLoc");
						real_time_loc[i].location=atoi(value);
						real_time_loc[i].length=2;
					}
					else if(!strcmp(value,"REAL"))
					{
						value=xmlGetProp(sensorcur,"dataLoc");
						real_time_loc[i].location=atoi(value);
						real_time_loc[i].length=4;
					}
					else if(!strcmp(value,"WORD"))
					{
						value=xmlGetProp(sensorcur,"dataLoc");
						real_time_loc[i].location=atoi(value);
						real_time_loc[i].length=2;
					}
					i++;
				}
				sensorcur=sensorcur->next;
			}
		}
		cur=cur->next;
	}
}

void msg_init()
{
	int msgid;
	msgid=msgget((key_t)MSGID_REMOTE,0666|IPC_CREAT);
	if(msgid==-1)
	{
		printf("cannot open msg\n");
		pthread_exit((void*)1);
	}
	printf("local:msg_remote init\n");
	remote_msgid=msgid;
	msgid=msgget((key_t)MSGID_LOCAL,0666|IPC_CREAT);
	if(msgid==-1)
	{
		printf("com msgget error\n");
		pthread_exit((void*)1);
	}
	local_msgid=msgid;
	printf("local:msg_local init\n");
	return;
}
/*
int gps_send(char gps_buff[][20])
{
	struct msg_local data;
	if(!strncmp(gps_buff[1],"A",1))
	{
		data.mtype=1;
		data.n_s=(char)gps_buff[3][0];
		strcpy(data.value1,gps_buff[2]);
		data.e_w=(char)gps_buff[5][0];
		strcpy(data.value2,gps_buff[4]);
	//	printf("n_s=%c,value1=%s,e_w=%c,value2=%s\n",data.n_s,data.value1,data.e_w,data.value2);
	}
	else
	{
		data.mtype=1;
		data.n_s='0';
		strcpy(data.value1,"0");
		data.e_w='0';
		strcpy(data.value2,"0");
	}
	if(msgsnd(local_msgid,(void*)&data,42,0)==-1)
	{
		printf("com msgsnd failed\n");
		return 0;
	}
	printf("	send gps ok\n");
	return 1;
}

int init_serial(void)
{
	serial_fd=open(DEVICE,O_RDWR|O_NOCTTY|O_NDELAY);
	if(serial_fd<0)
	{
		printf("com open device error\n");
		return -1;
	}
//	printf("serial_fd=%d\n",serial_fd);
//	if(serial_fd<0)
//	{
//		perror("com open");
//		return -1;
//	}
	struct termios options;
	tcgetattr(serial_fd,&options);//获得相关初始参数
	options.c_cflag|=(CLOCAL|CREAD);//本地链接 接收使能
	options.c_cflag&=~CSIZE;//需要屏蔽
	options.c_cflag&=~CRTSCTS;//无硬件流控
	options.c_cflag|=CS8;//8位数据位
	options.c_cflag&=~CSTOPB;//1位停止位
	options.c_iflag&=~PARENB;//无奇偶校验位
	options.c_iflag|=IGNCR;
	//options.c_cc[VTIME]=0;
	//options.c_cc[VMIN]=40;
	options.c_oflag=0;//输出模式
	options.c_lflag=ICANON;//规范模式//options.c_lflag=0;//不激活终端模式
	cfsetospeed(&options,BOUDRATE);//设置波特率
	tcflush(serial_fd,TCIFLUSH);//溢出数据可以接收 但不读
	tcsetattr(serial_fd,TCSANOW,&options);//所有改变立即生效
	printf("	com serial init ok\n");
	return 0;
}

int uart_send(char*data,int datalen)
{
	int len=0;
	len=write(serial_fd,data,datalen);
	if(len==datalen)
	{
		return len;
	}
	else 
	{
		tcflush(serial_fd,TCOFLUSH);
		return 0;
	}
	return 0;
}

int uart_recv(char*data,int datalen)
{
	int i=0,len,ret=0;
	fd_set fs_read;
	struct timeval tv_timeout;
	char temp[1024];
	FD_ZERO(&fs_read);
	FD_SET(serial_fd,&fs_read);
	tv_timeout.tv_sec=12;
	tv_timeout.tv_usec=0;
	ret=select(serial_fd+1,&fs_read,NULL,NULL,&tv_timeout);
//	printf("ret=%d\n",ret);
	if(FD_ISSET(serial_fd,&fs_read))
	{
		len=read(serial_fd,temp,512);
		if(len>0)
		{
			temp[len]='\0';
			strcpy(data,temp);
			return len;
		}
		if(len<=0)
		{
			return 0;
		}
	}
	else
	{
		perror("com select");
		return 0;
	}
	return 0;
}

int gps_deal(char*data,char gps_buff[][20])
{
	char buf[1024];
	struct msg_gps mysend;
	int msgid;
	int i=0,j=0;
	char temp[12][20];
	int ret;
	char *p;
	strcpy(buf,data);
	ret=gps_check(buf);
	if(ret==0)
		return 0;
	if(strncmp(buf,"$GPRMC",6)!=0)
	{
		perror("com strncmp:$GPRMC");
		return 0;
	}
	p=strchr(buf,',');
	p++;
	while(*p!='*')
	{
		if(*p==',')
		{
			gps_buff[i][j]='\0';
			i++;
			j=0;
		}
		else
		{
			gps_buff[i][j]=*p;
			j++;
		}
		p++;
	}
	if(strncmp(gps_buff[1],"A",1)==0)
	{
		printf("gps is valid\n");
	//	return 1;

	}
//	printf("gps deal ok\n");
	return 1;
}

int gps_check(char*data)
{
	unsigned char c,h,l;
	char*p;
	char chk[3];
	p=data;
	if(*p!='$')
	{
		printf("com check error\n");
		return 0;
	}
	p++;
	c=(unsigned char)*p;
	p++;
	for(;*p!='*';++p)
	{
		c^=(*p);
	}
	h=c&0xf0;
	h=h>>4;
	l=c&0x0f;
	if(h<10)
		chk[0]=h+'0';
	else
		chk[0]=h-10+'A';
	if(l<10)
		chk[1]=l+'0';
	else
		chk[1]=l-10+'A';
	chk[2]='\0';
	p=strchr(data,'*');
	p++;
	if(!strncmp(p,chk,2))
	{
		printf("	com check ok\n");
		return 1;
	}
	return 0;
}
*/
void* first_level_recv()
{
	int rc;
	struct msg_local first_data;
	unsigned char data[MSG_MAX];
	first_data.mtype=3;
    int udp_socket;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t sin_size;
	int num;
	udp_socket=socket(AF_INET,SOCK_DGRAM,0);
	bzero(&server,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(UDP_PORT);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	bind(udp_socket,(struct sockaddr*)&server,sizeof(struct sockaddr));
	sin_size=sizeof(struct sockaddr_in);
	printf("local:first level recv pthread\n");
	while(1)
	{
		num=recvfrom(udp_socket,first_data.data,MSG_MAX,0,(struct sockaddr*)&client,&sin_size);
		if(num<0)
		{
			perror("recv error:");
			exit(1);
		}
//		printf("1\n");
		first_data.length=num;
//		printf("num=%d\n",num);
		if((rc=msgsnd(local_msgid,&first_data,(MSG_MAX+4),0))==-1)
		{
			printf("local msgsnd failed!\n");
			exit(1);
		}
//		printf("send msg data ok \n");
		sleep(1);
	}
	pthread_exit((void*)1);
}

int read_plcfile()
{
	int i;
//	struct plc_struct *q=NULL;
	int number;
	int length;
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
	org_id=read_profile(part1,part2);
//	printf("%2x\n",org_id);
	strcpy(part1,"number_area");
	strcpy(part2,"number");
	number=(int)read_profile(part1,part2);
//	printf("%d\n",number);
//	printf("number=%d org_id=%2x\n",number,org_id);
	for(i=1;i<=number;i++)
	{
		struct plc_struct* newnode;
		newnode=(struct plc_struct*)malloc(sizeof(struct plc_struct));
		newnode->org_id=org_id;
		sprintf(part1,"%d",i);
		strcpy(part2,"area");
		dbnr=read_profile(part1,part2);
	//	printf("1\n");
		newnode->dbnr=dbnr;
		strcpy(part2,"start_address_h");
		start_address_h=read_profile(part1,part2);
		newnode->start_address_h=start_address_h;
		strcpy(part2,"start_address_l");
		start_address_l=read_profile(part1,part2);
		newnode->start_address_l=start_address_l;
		strcpy(part2,"len_h");
		len_h=read_profile(part1,part2);
		newnode->len_h=len_h;
		strcpy(part2,"len_l");
		len_l=read_profile(part1,part2);
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
//	q=plc_head;
//	for(;q!=NULL;q=q->next)
//	{
//		printf("%2x\n",q->org_id);
//		fflush(stdout);
//		printf("%02x\n",q->dbnr);
//	}
	return 1;
}

void *second_level_recv()
{
	int flag;
	unsigned char c;
	unsigned char real_time[256];
	unsigned char second_data[6*1024];
	struct plc_struct *p;
	struct msg_remote sdata;
	int semid,shmid;
	int rc;
	int count;
	int fd;
	int init;
	int pipe_fd;
	int length;
	char*shmaddr;
	char*ret;
	struct shm_local* shared;
	printf("local:second level recv pthread\n");
	fd=tcp_connect();
	read_plcfile();
	length=sizeof(struct msg_remote)-sizeof(long);
	if((shmid=creatshm(1))==-1)
		exit(1);
	if((shmaddr=shmat(shmid,(char*)0,0))==(char*)-1)
	{
		perror("attch shared memory error\n");
		exit(1);
	}
	shared=(struct shm_local *)shmaddr;
	shared->written=0;
	if((semid=creatsem(1))==-1)
	{
		DEBUG("creatsem error");
		exit(1);
	}
	if(access(FIFO_NAME,F_OK)==-1)
	{
		if((mkfifo(FIFO_NAME,0666)<0)&&(errno!=EEXIST))
		{
			printf("cannot create fifo\n");
			exit(1);
		}
	}
	while(1)
	{
		flag=0;
		switch(real_time_data)
		{
			case 0:
			if(init==1)
			{
//				printf("normal\n");
				c=0x00;
				rc=write(pipe_fd,&c,1);
				if(rc!=1)
				{
					DEBUG("write error");
					exit(1);
				}
				init=0;
				close(pipe_fd);
				printf("local:close pipe\n");
			}
			length=tcp_receive(fd,second_data);
		//	printf("length=%d\n",length);
			while(1)
			{
				sem_p(semid);
				if(shared->written==0)
				{
					memcpy(shared->data,second_data,length);
					shared->length=length;
					shared->written=1;
				//	printf("	send shm data ok\n");
					flag=1;
				}
				sem_v(semid);
				if(flag==1)
				{
					sleep(second_rate);
					break;
				}
				else
				{
					usleep(100000);
					continue;
				}
			}
			break;
			case 1:
			if(!init)
			{
				pipe_fd=open(FIFO_NAME,O_WRONLY);
				if(pipe_fd==-1)
				{
					DEBUG("open pipe error");
					exit(1);
				}
				init=1;
			}
			printf("local:real_time\n");
			length=tcp_receive(fd,second_data);
			length=real_time_select(second_data,real_time);
			if(length<=0)
			{
				DEBUG("select error");
				exit(1);
			}
			c=(unsigned char)length;
			rc=write(pipe_fd,&c,1);
//			printf("rc=%d\n",rc);
			if(rc!=1)
			{
				DEBUG("write error:%d",rc);
				exit(1);
			}
			rc=write(pipe_fd,real_time,length);
			if(rc<=0)
			{
				DEBUG("write error");
				exit(1);
			}
//			printf("rc=%d,length=%d\n",rc,length);
			usleep(100000);
			break;
		}
	}
	exit(1);

}

int real_time_select(unsigned char*data,unsigned char *real)
{
	int simucount=0;
	int bitcount=0;
	unsigned char *p;
	int i=0;
	unsigned int value=0;
	unsigned char simu[128];
	int bito[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
	for(;real_time_loc[i].used!=0;i++)
	{
		switch(real_time_loc[i].length)
		{
			case 0:
				if(*(p+real_time_loc[i].location)&bito[real_time_loc[i].offset])
					value=value|(1<<bitcount);
				bitcount++;
				break;
			case 2:
				memcpy(simu+simucount,data+real_time_loc[i].location,real_time_loc[i].length);
				simucount+=2;
				break;
			case 4:
				memcpy(simu+simucount,data+real_time_loc[i].location,real_time_loc[i].length);
				simucount+=4;
			break;
		}
	}
	memcpy(real,&value,4);
	p=real;
	memcpy(p+4,simu,simucount);
	return (simucount+4);
}

int tcp_receive(int fd,unsigned char *data)
{
	int i;
	int rc;
	int length=0;
	struct fetch_res *q;
	struct plc_struct *p;
	unsigned char temp[PLC_LEN];
	p=plc_head;
	struct fetch request;
	request.systemid_1=0x53;
	request.systemid_2=0x35;
	request.len_of_head=0x10;
	request.id_op_code=0x01;
	request.len_op_code=0x03;
	request.op_code=0x05;
	request.org_field=0x03;
	request.len_org_field=0x08;
	request.empty_field=0xff;
	request.len_empty_field=0x02;
	while(p!=NULL)
	{
//		printf("1\n");
		request.org_id=p->org_id;
		request.dbnr=p->dbnr;
		request.start_address_h=p->start_address_h;
		request.start_address_l=p->start_address_l;
		request.len_h=p->len_h;
		request.len_l=p->len_l;
	//	printf("%02x,%02x,%02x,%02x,%02x,%02x\n",request.org_id,request.dbnr,request.start_address_h,request.start_address_l,request.len_h,request.len_l);
			
		rc=write(fd,&request,sizeof(struct fetch));
		if(rc!=sizeof(struct fetch))
		{
			DEBUG("write error");
			exit(1);
		}
		rc=read(fd,temp,REQUEST);
	//	for(i=0;i<rc;i++)
	//		printf("%02x ",temp[i]);
	//	printf("rc=%d\n",rc);
		if(rc==16)
		{
			q=(struct fetch_res*)temp;
			if(q->error_field==0x00)
			{
				rc=read(fd,temp,PLC_LEN);
	//			for(i=0;i<rc;i++)
	//				printf("%02x ",temp[i]);
				if(rc<=0)
				{
					DEBUG("recv error:%d",rc);
					exit(1);
				}
				memcpy((data+length),temp,rc);
				length+=rc;
			}
			else
				return -1;
		}
		else
		{
			DEBUG("recv error:%d",rc);
			exit(1);
		}
		p=p->next;
	}
//	printf("length=%d\n",length);
	return length;
}

void main()
{
	int err;
	FILE*fp;
	msg_init();
	sigset_t set;
	pthread_t tid1;
	pthread_t tid2;
	pthread_t tid3;
	write_pid_local();
//	printf("1\n");
	pthread_attr_t attr;
	sigemptyset(&set);
	sigaddset(&set,SIGUSR1);
	sigaddset(&set,SIGUSR2);
	err=pthread_sigmask(SIG_BLOCK,&set,NULL);
	if(err!=0)
		exit(1);
	err=pthread_attr_init(&attr);
	if(err!=0)
		exit(1);
	err=pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if(err==0)
	{
		err=pthread_create(&tid1,&attr,first_level_recv,NULL);
		if(err!=0)
			exit(1);
		err=pthread_create(&tid2,&attr,second_level_recv,NULL);
		if(err!=0)
			exit(1);
		err=pthread_create(&tid3,&attr,signal_wait,NULL);
		if(err!=0)
			exit(1);
		pthread_attr_destroy(&attr);
	}
	pthread_exit((void*)1);
}
