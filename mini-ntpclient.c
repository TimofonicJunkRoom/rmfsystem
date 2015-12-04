/* Stripped-down & simplified ntpclient by tofu
 *
 * Copyright (C) 1997-2010  Larry Doolittle <larry@doolittle.boa.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License (Version 2,
 *  June 1991) as published by the Free Software Foundation.  At the
 *  time of writing, that license was published by the FSF with the URL
 *  http://www.gnu.org/copyleft/gpl.html, and is incorporated herein by
 *  reference.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include "mini-ntpclient.h"

#include "rmfsystem.h"
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/timex.h>
#include "read_file.h"
 
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
	read_file("NTP","ntpaddress",value);
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

/**
 * Local Variables:
 *  compile-command: "make mini-ntpclient"
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
