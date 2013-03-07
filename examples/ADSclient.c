/*
 Sample client using BECKHOFF's ADS protocol. 
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System Nov 2002.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of BECKHOFF 
 Company. www.beckhoff.de

 (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2003, 2005.

 This is free software; you can redistribute it and/or modify
 it under the terms of the GNU Library General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This Software is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  
*/

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#include "ads.h"
#include "AdsDEF.h"

#include <sys/time.h>

#define ThisModule "ADSclient : "

#define debug 10

#define ADSPtEmpty -2
#define ADSPtMPIAck -3
#define ADSPtUnknownMPIFunc -4
#define ADSPtUnknownPDUFunc -5
#define ADSPtReadResponse 1
#define ADSPtWriteResponse 2

AMSNetID me = { 127, 0, 0, 1, 1, 1 };
AMSNetID other = { 127, 0, 0, 1, 1, 1 };

void readIndexGroup(ADSConnection * dc, int igr)
{
	int res;
	printf("Trying to read from index group %04x\n", igr);
	res = ADSreadBytes(dc, igr, 0, 20, NULL);
	if (res == 0)
		_ADSDump("data", dc->dataPointer, dc->AnswLen);
	else
		printf(" Error %s ", ADSerrorText(res));
	printf("\n");
}

int main(int argc, char **argv)
{
//    ADSDebug=ADSDebugAll;

//    ADSDebug=ADSDebugAnalyze;
	_ADSOSserialType fds;
	ADSInterface *di;
	ADSConnection *dc;
	struct timeval t1, t2;
	int netFd;
	int i, apn, k, port;
	FILE *logFile;
	uc buffer[maxDataLen];
	int res;
	uc *b;
	struct sockaddr_in addr;
	struct hostent *he;
	int log = 0;
	int opt;
	socklen_t addrlen;
	AdsVersion Version;
	PAdsVersion pVersion = &Version;
	char pDevName[16];
	unsigned short ADSstate, devState;

	if (argc < 3) {
		printf("Usage: ADSclient host port \n");
		printf("Example: ADSclient 192.168.17.110 48898\n");
		return -1;
	}

	port = atol(argv[2]);

	ads_debug(ADSDebug,"host: %s port %d\n", argv[1], port);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_aton(argv[1], &addr.sin_addr);
	netFd=socket(AF_INET, SOCK_STREAM, 0);
	if (errno != 0) {
		ads_debug(ADSDebug, ThisModule "socket %s\n",
			  strerror(errno));
	}
	if (netFd <= 0) {
		printf("Could not connect to host!\n");
		return -1;
	}
	ads_debug(ADSDebug,"netFd: %d\n", netFd);
	
	
	
	addrlen = sizeof(addr);
	
	if (connect(netFd, (struct sockaddr *) & addr, addrlen)) {
		ads_debug(ADSDebug,ThisModule "Socket error: %s \n", strerror(errno));
		close(netFd);
		return 0;
    	} else {
		ads_debug(ADSDebug,ThisModule "Connected to host: %s \n", argv[1]);
	} 
	
	errno=0;
	opt = 1;
	res=setsockopt(netFd, SOL_SOCKET, SO_KEEPALIVE, &opt, 4);
	//BUG HERE: ads_debug(ADSDebug, ThisModule "setsockopt %s\n", strerror(errno));
	fds.rfd = netFd;
	fds.wfd = netFd;
	di = ADSNewInterface(fds, me, 800, "test");
	dc = ADSNewConnection(di, other, 800);
	b = buffer;
	ads_debug(ADSDebug,"device info:\n");

	ADSreadDeviceInfo(dc, &pDevName, pVersion);
	ads_debug(ADSDebug,"read state:\n");
//    ADSreadBytes(dc,igr,0,100,NULL);
//    ADSwriteBytes(dc,igr,0,100,NULL);
	ADSreadState(dc, &ADSstate, &devState);
	//ads_debug(ADSDebug,"write control:");
	ADSwriteControl(dc, 4, 0, NULL, 0);

	readIndexGroup(dc, 0x4020);
	readIndexGroup(dc, 0x4021);
//    readIndexGroup(dc, 0x4025);
	readIndexGroup(dc, 0x4030);
	readIndexGroup(dc, 0x4031);
	readIndexGroup(dc, 0x4040);

	readIndexGroup(dc, 0xf003);
	readIndexGroup(dc, 0xf004);
	readIndexGroup(dc, 0xf020);
	readIndexGroup(dc, 0xf021);
	readIndexGroup(dc, 0xf030);
	readIndexGroup(dc, 0xf031);
	readIndexGroup(dc, 0xf060);
//    ADSaddDeviceNotification(dc, igr, 0, 10, ADS_TRANS_NOTRANS, 100000, 120000);
	/* something is missing here TODO 
	ADSaddDeviceNotification(dc, 0x4020, 0, 10, ADS_TRANS_CLIENTCYCLE,
				 100000, 120000);
	*/
//    ADSaddDeviceNotification(dc, igr, 0, 10, ADS_TRANS_CLIENT1REQ, 100000, 120000);
//    ADSaddDeviceNotification(dc, igr, 0, 10, ADS_TRANS_SERVERCYCLE, 100000, 120000);
//    ADSaddDeviceNotification(dc, igr, 0, 10, ADS_TRANS_SERVERONCHA, 100000, 120000);
//    ADSaddDeviceNotification(dc, 0x801f, 0, 10, 6, 100000, 120000);
//	ADSaddDeviceNotification(dc, 0x4020, 0, 10, ADSTRANS_SERVERONCHA,
//				 100000, 120000);
/*    
    k=0x4000;
    for (i=k; i<k+60; i++)
	ADSreadBytes(dc,i,0,100,NULL);
//    double usec = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec)*1e-6;
*/
	return 0;
}

