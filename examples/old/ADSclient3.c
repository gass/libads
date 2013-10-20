/*
    libads is an implementation of the Beckhoff's ADS protocol.

    libads is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libads is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    Lesser GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with libads.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ads.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "log2.h"

#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <byteswap.h>

//#define DONT_USE_GETHOSTBYNAME

#define bSize 1256
#define us unsigned short

//#include "openSocket.cc"

#include "openSocket.h"
#include <sys/ioctl.h>
#ifndef DONT_USE_GETHOSTBYNAME
#include <netdb.h>
#endif

int ADSDebug;

#define debug 10

#define ADSPtEmpty -2
#define ADSPtMPIAck -3
#define ADSPtUnknownMPIFunc -4
#define ADSPtUnknownPDUFunc -5
#define ADSPtReadResponse 1
#define ADSPtWriteResponse 2

int _ADSwrite22(ADSInterface * di, void *buffer, int len)
{
	int res;
	if (di == NULL)
		return -2;
	if (di->error)
		return -1;
	_ADSDump("I send: ", buffer, len);
	res = write(di->fd.wfd, buffer, len);
	if (res < 0)
		di->error |= 1;
	return res;
}

AMSNetID me = { 172, 16, 17, 5, 1, 1 };
AMSNetID other = { 172, 16, 17, 1, 1, 1 };

void setupHeader(AMSheader * h)
{
/*
typedef struct _AMSheader {
    AMSNetID	targetId;
    us		targetPort;
    AMSNetID	sourceId;
    us		sourcePort;
    us		commandId;
    us		stateFlags;
    u32 	dataLength;
    u32 	errorCode;
    u32 	invokeId;	//user-defined 32-bit field. Usually it is used to identify
				// a response as belonging to a certain request sent with the 
} AMSheader;
*/
	h->targetId = other;
	h->targetPort = 800;
	h->sourceId = me;
	h->sourcePort = 0x8067;
	h->commandId = 2;
	h->stateFlags = 0x404;
//    h->stateFlags=0;
	h->dataLength = 12;
	h->errorCode = 0;
	h->invokeId = 0x1237;	//user-defined 32-bit field. Usually it is used to identify
};

/*
typedef struct _ADSpacket {
    ADS_TCPheader	adsHeader;
    AMSheader		amsHeader;
    char data[maxDataLen];
} ADSpacket;
*/

void ADSsend(ADSConnection * dc)
{
	ADSpacket *p = (ADSpacket *) (dc->msgOut);
	_ADSwrite22(dc->iface, dc->msgOut, p->adsHeader.length + 6);
}

int main(int argc, char **argv)
{
	uc test[] = { 0x71, 0x72, 0x73, 0x24, 0x25 };
//    ADSDebug=ADSDebugAll &~ADSDebugByte &~ADSDebugPacket;
//    ADSDebug=0;
	ADSDebug = ADSDebugAll;
	_ADSOSserialType fds;
	ADSInterface *di;
	ADSConnection *dc;
	AMSheader *h1;
	AMS_TCPheader *h2;
	ADSpacket p1;
	int port, ki;
	char *peer;

	struct timeval t1, t2;
	int log = 0;
	int netFd, i, apn;
	FILE *logFile;

	h1 = &(p1.amsHeader);
	h2 = &(p1.adsHeader);

	if (argc < 3) {
		printf("Usage: ADSclient host:port indexgroup \n");
		printf
		    ("Example: ADSclient 192.168.17.110 1099 16433\nI don't remeber which numbers where valid index groups.\n");
		return -1;
	}
	if (argc >= 4) {
		logFile = fopen(argv[3], "w+");
		if (logFile == NULL) {
			printf("Could not open log file!\n");
			return -1;
		}
		log = 1;
		printf("Logging to %s\n", argv[3]);
	}
	uc buffer[bSize];
	peer = argv[1];
	port = atol(argv[2]);

	int igr = atol(argv[2]);

	_ADSDumpAMSNetId(&me);
	_ADSDumpAMSNetId(&other);
	setupHeader(h1);
	_ADSDumpAMSheader(h1);

	p1.adsHeader.length = 44;
	p1.adsHeader.reserved = 0;

	ADSreadRequest *rq = (ADSreadRequest *) & p1.data;
//    rq->indexGroup=0xF021;//igADSParameter;
//    rq->indexGroup=igADSParameter;
	rq->indexGroup = igr;
	rq->indexOffset = 0;
	rq->length = 400;
	LOG2("Index Group:   %d\n", rq->indexGroup);
	LOG2("Index Offset:  %d\n", rq->indexOffset);
	LOG2("Data length:  %d\n", rq->length);
//    _ADSDump("Data: ", rq->data, rq->length);

	LOG3("host: %s port %d\n", peer, port);
	netFd = openSocket(port, peer);
	if (netFd <= 0) {
		printf("Could not connect to host!\n");
		return -1;
	}
	LOG2("netFd: %d\n", netFd);
	fds.rfd = netFd;
	fds.wfd = netFd;

	di = ADSNewInterface(fds, me, 300, "TEST");
	dc = ADSNewConnection(di, other, 800);

//    _ADSwrite(dc->iface, &p1, p1.adsHeader.length+6);
	_ADSwrite22(dc->iface, (uc *) & p1, p1.adsHeader.length + 6);
	int res;
	uc *b = buffer;

//    ADSConnectPLC_IBH(dc);

	gettimeofday(&t1, NULL);

	for (ki = 0; ki < 2; ki++) {
/*    
//	ADSWriteBytesIBH(dc, ADSDB, 40, 0, 5, &test);
//	ADSReadBytesIBH(dc, ADSDB, 40, 0, 100, NULL);
	_ADSDump("DB40:",(uc*)(dc->dataPointer),5);
//	ADSReadBytesIBH(dc, ADSDB, 11, 0, 100, NULL);
	_ADSDump("DB11:",(uc*)(dc->dataPointer),dc->AnswLen);
*/
		dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn);
		if (dc->AnswLen > 0) {
			res = dc->AnswLen;
			_ADSDump("packet", dc->msgIn, dc->AnswLen);
		}

	}
	gettimeofday(&t2, NULL);
	double usec =
	    (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) * 1e-6;
	printf(" %g sec.\n", usec);
	FLUSH;
//    ADSDisconnectPLC_IBH(dc);
	return 0;

}

/*
    Changes:
    
    05/12/2002 created
    23/12/2002 added sys/time.h
*/
