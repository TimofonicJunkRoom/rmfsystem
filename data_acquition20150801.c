/*************************************************************************
  > File Name: data_acqusition.c
  > Author: xh
  > Mail: banre123@126.com 
  >20150708:change to two pthread
  >20150715:add mesg queue and share memory
  >20150722:add signal process
  >20150801:add process id config and data acquisition frequency config
  > Created Time: Wed 08 July 2015 08:34:07 PM CST
 ************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include<string.h>
#include<sys/msg.h>
#include<signal.h>
#include<fcntl.h>

#define HOST_ADDRESS "127.0.0.1"
#define PLC_ADDRESS "127.0.0.1"
#define TCP_PORT 2000
#define UDP_PORT 2003
#define COMM_CONF "config"
#define RAW_DATA "rawdata"
#define PID_CONF "pid_conf"
#define ACQUI_CONF "acqui_conf"
#define MAXLINE 4096
#define MESG_KEY 1000	//System V mesg queue key
#define UDP_LEN 240
#define SHM_KEY 2000 // System V share memory key

size_t shm_len=100000;//System V share memory length
unsigned char is_signal_flag=0;//real time data tran flag

int tcp_connect();
int udp_connect();

int tcp_acqui();
int udp_acqui();

void *thread_tcp(void *);//tcp thread function
void *thread_udp(void *);//udp thread function
void *thread_sigprocess(void *);// signal handle thread function

int msg_init();
int shm_init(int);

//static void sig_usr(int);// onr handler for both signals
sigset_t mask;	//
int main()
{	
	pthread_t tcp_tid,udp_tid,sigprocess_tid;
	int err1,err2,err;
	sigset_t oldmask;
	pid_t pid;
	FILE *fp;
	char buf[4096];
	//int len;
	printf("data_acquisition process id is %d\n",pid=getpid());
	fp=fopen(PID_CONF,"r+");//need created file before this program start!!!!!!!!!!
	if ( flock(fileno(fp), LOCK_EX) < 0)
	{
		perror("lock pid conf error");
		exit(1);
	}
	fgets(buf,4096,fp);//first line
	//	len=strlen(buf)
	fgets(buf,4096,fp);
	fprintf(fp,"local pid:%d\nremote pid:%s\n",pid,buf);
	fflush(fp);
	//	fclose(fp);
	if ( flock(fileno(fp), LOCK_UN) < 0)
	{
		perror("unlock pid conf error");
		exit(2);
	}
	fclose(fp);//before or after unlock ??????
	/* block SIGUSR1 and SIGUSR2 in main thread, establish signal handlerin the special thread to handle the siganl 	*/
	sigemptyset(&mask);
	sigaddset(&mask,SIGUSR1);
	sigaddset(&mask,SIGUSR2);
	if((err=pthread_sigmask(SIG_BLOCK,&mask,&oldmask))!=0)
		perror("SIG_BLOCK error");
	err=pthread_create(&sigprocess_tid,NULL,thread_sigprocess,NULL);
	if(err!=0)
		perror("can't create sigprocess thread");

	err1=pthread_create(&udp_tid,NULL,thread_udp,NULL);
	if(err1!=0)
		perror("cant't create udp thread ");
	err2=pthread_create(&tcp_tid,NULL,thread_tcp,NULL);
	if(err2!=0)
		perror("can't create tcp thread");

	//	pause();hang up the whole process
	while(1);// main thread will not exit ,keep tcp and udp thread run

	/*	if(signal(SIGUSR1,sig_usr1)==SIG_ERR)
		err_sys("can't catch SIGUSR1\n");

		if(signal(SIGUSR2,sig_usr2)==SIG_ERR)
		err_sys("can't catch SIGUSR2\n");
		*/		
}

int tcp_connect()
{

	int client_sockfd;
	int client_len;
	struct sockaddr_in client_address;
	int result,i;
	client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	client_address.sin_family = AF_INET;
	client_address.sin_addr.s_addr = inet_addr(PLC_ADDRESS);
	client_address.sin_port =htons(TCP_PORT);
	client_len = sizeof(client_address);
	result = connect(client_sockfd, (struct sockaddr *)&client_address,client_len);
	while(i<=20)
	{	
		if(result!=-1)
			break;
		result = connect(client_sockfd, (struct sockaddr *)&client_address,client_len);
		sleep(++i*5);
	}

	if(result == -1) {
		perror("oops: client1");
		exit(1);
	}
	printf("CONNECT PLC OK!\n");
	return client_sockfd;
}

int udp_connect()
{

	int server_sockfd;
	struct sockaddr_in servaddr,cliaddr;
	server_sockfd=socket(AF_INET,SOCK_DGRAM,0);
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET; 
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(UDP_PORT);
	bind(server_sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	return server_sockfd;
	//unsigned char mesg[MAXLINE];
	//	recvfrom(server_sockfd,mesg,MAXLINE,0,NULL,0);
	//	printf("%s\n",mesg);
}

void *thread_tcp(void *arg)
{
	FILE *fp,*fp1;
	int tcp_sockfd;
	int data_frequency=3;
	int shm_id;
	shm_id=shm_init(shm_len);
	tcp_sockfd=tcp_connect();
	while(1){

		unsigned char tcp_data[4096];
		unsigned char req_buf[16]={0x53,0x35,0x10,0x01,0x03,0x05,0x03,0x08,0x01,0x01,0x00,0x00,0x00,0x05,0xff,0x02};//DB1 addr(0-2047)  
		int rn,wn,i;
		int cmd=0;
		int fre_level;
		char buf[MAXLINE];
		//	tcp_sockfd=tcp_connect();// establish to much connection
	/*	if((fp=fopen(ACQUI_CONF,"r"))==NULL)
		{
			perror("open acquisition config error\n");
		}
	*/
		//fgets(fp,)
		/*	fp1=fopen(RAW_DATA,"ab+");
			if(fp1==NULL)
			{
			perror("fopen error");
			}	
			*/
		if(is_signal_flag!=0)
		{
			
			if((fp=fopen(ACQUI_CONF,"r"))==NULL)
			{
			perror("open acquisition config error\n");
			}
			if ( flock(fileno(fp), LOCK_EX) < 0)
			{
				perror("lock pid conf error");
				exit(1);
			}
			fgets(buf,4096,fp);//first line 0/1/2 0:means normal mode,default freq,1 means real time tran mode,2 means new frequency
			cmd=buf[0]-'0';
			fgets(buf,4096,fp);// second line : level of data acquisition frequency
			fre_level=buf[0]-'0';
			if ( flock(fileno(fp), LOCK_UN) < 0)
			{
				perror("unlock pid conf error");
				exit(2);
			}
			is_signal_flag=0;
		}
		wn=write(tcp_sockfd,req_buf,16);
		switch(cmd){
			case 0:
				sleep(data_frequency);
				break;
			case 1:
				sleep(data_frequency*fre_level)
				break;
			case 2:
				break;
			default:
				printf("unknown command :%d\n",cmd);
		}
		rn=read(tcp_sockfd,tcp_data,4096);
		int wc;
		//wc=fwrite(tcp_data+16,rn-16,fp1);
		//fflush(fp1);
		for(i=0;i<rn;i++)
		{
			printf("%02x ",tcp_data[i]);
			fflush(stdout);
		}

		/*data to do */
	}
}

void *thread_udp(void *arg)
{

	int i,j;
	int udp_fd;
	struct msgbuf
	{
		long mtype;
		unsigned char udpdata[UDP_LEN];
	}udp_msg;//udp mesg queue
	unsigned char data[UDP_LEN];//UDP DATA
	udp_fd=udp_connect();
	j=mesg_init();
	while(1)
	{
		int len;
		recvfrom(udp_fd,data,UDP_LEN,0,NULL,0);//NULL means not care the client_addr,and the length
		len=strlen(data);
		udp_msg.mtype=1;
		for(i=0;i<UDP_LEN;i++)
			udp_msg.udpdata[i]=data[i];
		msgsnd(j,&udp_msg,UDP_LEN,0);
		data[len+1]='\0';
		printf("%s\n",data);
	}
}
void *thread_sigprocess(void *arg)
{

	int err,signo;
	while(1)
	{
		for(;;)
		{
			err=sigwait(&mask,&signo);
			if(err!=0)
				perror("sigwait failed");
			printf("debug 5\n");	
			switch(signo)
			{
				case SIGUSR1:
					is_signal_flag=1;
					//rtdata_flag=~(rtdata_flag|0x01); reverse ?
					printf("1"); 				//to do 
					fflush(stdout);
					break;
				case SIGUSR2:
					printf("2");//to do  
					fflush(stdout);
					break;
				default:
		 			printf("unexpected signal %d\n",signo);
					exit(1);
			}
		}

	}
}
int mesg_init()
{
	int megid;
	megid=msgget(MESG_KEY,IPC_CREAT);
	return megid;

}
int shm_init(int size)
{
	int shmid;
	shm_len=size;
	shmid=shmget(SHM_KEY,shm_len,IPC_CREAT);
	return shmid;
}
/*
   static void sig_usr(int signo)//argument is signal number
   {
   if(signo==SIGUSR1)
// do something SIGUSR1 trigger
else if(signo==SIGUSR2)
//do something SIGUSR2 trigger
else 
err_dump("receive signal %d\n",signo);
}
*/
