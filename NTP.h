#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <time.h>

#define uchar unsigned char
#define uint unsigned int

#define LEN 30 
#define From00to70  0x83aa7e80U
#define NTPPORT 123
#define SERVER "202.120.2.101"
typedef struct NTPPACK
{
	char li_vn_mode;
	char stratum;
	char pool;
	char precision;
	unsigned long root_delay;
	unsigned long root_dispersion;
	char ref_id[4];

	unsigned long reftimestamphigh;
	unsigned long reftimestamplow;

	unsigned long oritimestamphigh;
	unsigned long oritimestamplow;

	unsigned long recvtimestamphigh;
	unsigned long recvtimestamplow;

	unsigned long trantimestamphigh;
	unsigned long trantimestamplow;
}NTPPacket;

NTPPacket ntppack, newpack;

long long firsttimestamp, finaltimestamp;
long long diftime, delaytime;

void NTP_Init()
{
	bzero(&ntppack, sizeof(ntppack));
	ntppack.li_vn_mode = 0x1b;

	firsttimestamp = From00to70 + time(NULL); 
	ntppack.oritimestamphigh = htonl(firsttimestamp);
}


int time_init(void)
{
	printf("system time sync!......\n");
	fd_set inset1; 
	struct timeval tv, tv1;

	int sockfd; 
	struct sockaddr_in addr; 

//	struct timezone tz;

	char server[LEN];
	char *server_t;

	server_t = malloc(strlen(SERVER) + 1);
	strncpy(server_t, SERVER, strlen(SERVER));
//	printf("ip port server_t is : %s\n", server_t);

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("create socket error!\n");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(NTPPORT); 
	addr.sin_addr.s_addr = inet_addr(server_t); 
	bzero(&(addr.sin_zero), 8); 

	tv.tv_sec = 5; 
	tv.tv_usec = 0;

	FD_ZERO(&inset1);
	FD_SET(sockfd, &inset1);

	NTP_Init(); 
	sendto(sockfd, &ntppack, sizeof(ntppack), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr));

	if(select(sockfd + 1, &inset1, NULL, NULL, &tv) < 0)
	{
		perror("select error!\n");
		exit(1);
	}
	else
	{
	//	printf("select is OK\n");
		if(FD_ISSET(sockfd, &inset1))
		{
			if(recv(sockfd, &newpack, sizeof(newpack), 0) < 0)
			{
				perror("recv newpack error!\n");
				exit(1);
			}
		}
	}

	finaltimestamp = time(NULL) + From00to70;
	newpack.root_delay = ntohl(newpack.root_delay);
	newpack.root_dispersion = ntohl(newpack.root_dispersion);

	newpack.reftimestamphigh = ntohl(newpack.reftimestamphigh);
	newpack.reftimestamplow = ntohl(newpack.reftimestamplow);

	newpack.oritimestamphigh = ntohl(newpack.oritimestamphigh);
	newpack.oritimestamplow = ntohl(newpack.oritimestamplow);

	newpack.recvtimestamphigh = ntohl(newpack.recvtimestamphigh);
	newpack.recvtimestamplow = ntohl(newpack.recvtimestamplow);

	newpack.trantimestamphigh = ntohl(newpack.trantimestamphigh);
	newpack.trantimestamplow = ntohl(newpack.trantimestamplow);

	diftime = ((newpack.recvtimestamphigh - firsttimestamp) + (newpack.trantimestamphigh - finaltimestamp)) >> 1;
	
	delaytime = ((newpack.recvtimestamphigh - firsttimestamp) - (newpack.trantimestamphigh - finaltimestamp)) >> 1;
	tv1.tv_sec = time(NULL) + diftime + delaytime;
	tv1.tv_usec = 0;
	settimeofday(&tv1, NULL);
	system("hwclock -w");
	system("date");
	free(server_t);
	return 0;
}
