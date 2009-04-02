/*
 Sample client using BECKHOFF's ADS protocol. 
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System Nov 2002.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of BECKHOFF 
 Company. www.beckhoff.de

 (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2003.

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
//#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/time.h>

#include "log2.h"

#include "ads.h"

#define bSize 1256
#define us unsigned short

#include "openSocket.h"
//#include <sys/ioctl.h>



//#include <sys/socket.h>
#ifdef LINUX
#include <unistd.h>
#include <sys/time.h>
#include <net/ethernet.h>
#endif

#ifdef BCCWIN
#include <time.h>
//    void usage(void);
//    void wait(void);
#define WIN_STYLE
#endif


#define debug 10    

#define ADSPtEmpty -2
#define ADSPtMPIAck -3
#define ADSPtUnknownMPIFunc -4
#define ADSPtUnknownPDUFunc -5
#define ADSPtReadResponse 1
#define ADSPtWriteResponse 2

int _ADSwrite_i(ADSInterface * di, void * buffer, int len) {
    int res;
    if (di==NULL) return -2;
    if (di->error) return -1;
    _ADSDump("I write: ", buffer, len);
    res=write(di->fd.wfd, buffer, len);
    if (res<0) di->error|=1;
    return res;
}    

AMSNetID me   = {192,168,19,99,1,1};
AMSNetID other={10,242,56,5,1,1};

//AMSNetID other=	{97*0x1000000+ 19*0x10000+168*0x100+192,1,1};


uc eme[] ={0,0,0xF4,0xAC,0xD3,0x55};
uc eother[] ={0,1,5,0,0x35,0x2};

void setupHeader(AMSheader * h) {
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
    h->targetId=other;
    h->targetPort=301;
    h->sourceId=me;
    h->sourcePort=0x8067;
    h->commandId=2;
    h->stateFlags=4;
    h->dataLength=12;
    h->errorCode=0;
    h->invokeId=0x1237;	//user-defined 32-bit field. Usually it is used to identify
};

/*
typedef struct _ADSpacket {
    ADS_TCPheader	adsHeader;
    AMSheader		amsHeader;
    char data[maxDataLen];
} ADSpacket;
*/

void ADSsend(ADSConnection * dc) {
    ADSpacket * p=(ADSpacket *)(dc->msgOut);
//    _ADSwrite(dc->iface, dc->msgOut, p->adsHeader.length+6);
    _ADSwrite_i(dc->iface,dc->msgOut+12, p->adsHeader.length);
}

typedef struct _ADSpacketRaw {
    struct ethhdr	ethHeader;
    AMS_TCPheader	adsHeader;
    AMSheader		amsHeader;
    char data[maxDataLen];
} ADSpacketRaw;



int main(int argc, char **argv) {
    uc test[]={0x71,0x72,0x73,0x24,0x25};
    int ki, netFd,i,apn, log, port, igr;
//    ADSDebug=ADSDebugAll &~ADSDebugByte &~ADSDebugPacket;
//    ADSDebug=0;
    
    _ADSOSserialType fds;
    ADSInterface * di;
    ADSConnection * dc;
    AMSheader * h1;
    AMS_TCPheader * h2;
    struct ethhdr * h3;
    ADSpacketRaw p1;
    struct timeval t1, t2;
    FILE * logFile;
    uc buffer[bSize];
    
    ADSDebug=ADSDebugAll;
    h1= &(p1.amsHeader);
    h2= &(p1.adsHeader);
    h3= &(p1.ethHeader);
    log=0;
    
    if (argc<2) {
	printf("Usage: ADSclient  %d\n",argc);
	printf("Example: ADSclient 192.168.17.110:1099\n");
	return -1;
    }
    if (argc>=3) {
	logFile = fopen(argv[2],"w+");
	if (logFile==NULL) {
	    printf("Could not open log file!\n");
	    return -1;
	}
	log=1;
	printf("Logging to %s\n",argv[2]);
    }	
    

    port = atol(argv[2]);
    
    igr = atol(argv[3]);
    
    _ADSDumpAMSNetId(&me);
    _ADSDumpAMSNetId(&other);
    setupHeader(h1);
    _ADSDumpAMSheader(h1);
    
    h2->length=44;
    h2->reserved=0;
    
    ADSreadRequest * rq=(ADSreadRequest*) &p1.data;
//    rq->indexGroup=0xF021;//igADSParameter;
//    rq->indexGroup=igADSParameter;
    rq->indexGroup=igr;
    rq->indexOffset=0;
    rq->length=400;
    LOG2("Index Group:   %d\n", rq->indexGroup);
    LOG2("Index Offset:  %d\n", rq->indexOffset);
    LOG2("Data length:  %d\n", rq->length);
//    _ADSDump("Data: ", rq->data, rq->length);
    
    LOG3("host: %s port %d\n", argv[1], port);
    netFd=openSocket(port, argv[1]);
    if (netFd<=0) {
	printf("Could not connect to host!\n");
	return -1;
    }
    LOG2("netFd: %d\n", netFd);
    fds.rfd=netFd;
    fds.wfd=netFd;
    di=ADSNewInterface(fds,me, 300, "TEST");
    dc=ADSNewConnection(di, other, 800);
    
//    h3->h_dest=eother;
    memcpy(h3->h_dest,eother,6);
//    h3->h_source=eme;
    memcpy(h3->h_source,eme,6);	
//    h3->h_proto=0xBF01;
//    h3->h_proto=0x01BF;
//    h3->h_proto=0x8137;
//    h3->h_proto=0x88A4;    
    h3->h_proto=0xA488;

    _ADSwrite_i(dc->iface, &p1, 44+6+14 /*p1.adsHeader.length+6*/);
//    _ADSwrite(dc->iface, (uc*)(&p1)+6, p1.adsHeader.length);
    int res;
    uc *b =buffer;
    
//    ADSConnectPLC_IBH(dc);

    gettimeofday(&t1, NULL);


    for (ki=0;ki<2;ki++) {    
/*    
//	ADSWriteBytesIBH(dc, ADSDB, 40, 0, 5, &test);
//	ADSReadBytesIBH(dc, ADSDB, 40, 0, 100, NULL);
	_ADSDump("DB40:",(uc*)(dc->dataPointer),5);
//	ADSReadBytesIBH(dc, ADSDB, 11, 0, 100, NULL);
	_ADSDump("DB11:",(uc*)(dc->dataPointer),dc->AnswLen);
*/
	dc->AnswLen=_ADSReadPacket(dc->iface, dc->msgIn);
	if (dc->AnswLen>0) {
	    res=dc->AnswLen;
	    _ADSDump("packet", dc->msgIn, dc->AnswLen);
	}    
	
    } 
    gettimeofday(&t2, NULL);
    double usec = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec)*1e-6;
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
