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

#include "log2.h"

#include <fcntl.h>

#include "ads.h"

#ifdef BCCWIN
#include <time.h>
//    void usage(void);
//    void wait(void);
#define WIN_STYLE
#endif

#ifdef LINUX
#include <sys/time.h>
#define UNIX_STYLE
#endif

#include "openSocket.h"

#define debug 10

#define ADSPtEmpty -2
#define ADSPtMPIAck -3
#define ADSPtUnknownMPIFunc -4
#define ADSPtUnknownPDUFunc -5
#define ADSPtReadResponse 1
#define ADSPtWriteResponse 2

AMSNetID me = { 172, 16, 17, 1, 1, 1 };
AMSNetID other = { 172, 16, 17, 3, 1, 1 };

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
#ifdef UNIX_STYLE
	struct timeval t1, t2;
	int netFd;
#endif
#ifdef BCCWIN
	clock_t t1, t2;
	HANDLE netFd;
#endif
	int i, apn, k, port;
	FILE *logFile;
	uc buffer[maxDataLen];
	int res;
	uc *b;

	int log = 0;
	ADSDebug = 0;

	if (argc < 3) {
		printf("Usage: ADSclient host port \n");
		printf("Example: ADSclient 192.168.17.110 48898\n");
		return -1;
	}

	port = atol(argv[2]);

	LOG3("host: %s port %d\n", argv[1], port);
	netFd = openSocket(port, argv[1]);
	if (netFd <= 0) {
		printf("Could not connect to host!\n");
		return -1;
	}
	LOG2("netFd: %d\n", netFd);
	fds.rfd = netFd;
	fds.wfd = netFd;
	di = ADSNewInterface(fds, me, 800, "test");
	dc = ADSNewConnection(di, other, 800);
	b = buffer;

	LOG1("device info:\n");
	ADSreadDeviceInfo(dc);
	LOG1("read state:\n");
//    ADSreadBytes(dc,igr,0,100,NULL);
//    ADSwriteBytes(dc,igr,0,100,NULL);
	ADSreadState(dc, NULL, NULL);
	LOG1("write control:\n");
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
	ADSaddDeviceNotification(dc, 0x4020, 0, 10, ADS_TRANS_CLIENTCYCLE,
				 100000, 120000);
//    ADSaddDeviceNotification(dc, igr, 0, 10, ADS_TRANS_CLIENT1REQ, 100000, 120000);
//    ADSaddDeviceNotification(dc, igr, 0, 10, ADS_TRANS_SERVERCYCLE, 100000, 120000);
//    ADSaddDeviceNotification(dc, igr, 0, 10, ADS_TRANS_SERVERONCHA, 100000, 120000);
//    ADSaddDeviceNotification(dc, 0x801f, 0, 10, 6, 100000, 120000);
	ADSaddDeviceNotification(dc, 0x4020, 0, 10, ADSTRANS_SERVERONCHA,
				 100000, 120000);
/*    
    k=0x4000;
    for (i=k; i<k+60; i++)
	ADSreadBytes(dc,i,0,100,NULL);
//    double usec = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec)*1e-6;
*/
	return 0;
}

/*
    Changes:
    
    05/12/2002 created
    23/12/2002 added sys/time.h
*/
