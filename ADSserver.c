/*
 Sample server using BECKHOFF's ADS protocol. 
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#include "log2.h"

#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define ThisModule "ADSserver : "

#include "accepter.c"

#include "ADS.h"

#define debug 12


#ifdef LINUX
#include <byteswap.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#else    
#error Fill in what you need for your OS or API.
#endif

AMSNetID me   ={172,16,17,5,1,1};

AMSNetID partner  ={172,16,17,1,1,1};

/*
    many bytes. hopefully enough to serve any read request.
*/
uc dummyRes[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,
12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,12,13,14,1,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,
12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,12,13,14,1,
5,16,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,5,16,1,2,3,4,5,6,7,8,9,10,
11,12,13,14,15,16,12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,12,13,14,1,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,
12,13,14,15,16,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,12,13,14,1,
5,16,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,5,16,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,};
/*
    a read callback function
*/
uc * dummyRead (int area, int DBnumber, int start, int len, int * res) {
//    printf("User callback should deliver pointer to %d bytes from %s %d beginning at %d.\n",
//	len, ADSAreaName(area),DBnumber,start);
    *res=0;	
    
    return dummyRes;
};

void myWrite (int area, int DBnumber, int start, int len, int * res, uc*bytes) {
//    printf("User callback should write pointer to %d bytes from %s %d beginning at %d.\n",
//	len, ADSAreaName(area),DBnumber,start);
    *res=0;
    start=start/8;	
    memcpy(dummyRes+start,bytes,len);
};

int gpacketNumber=0;

/*
typedef struct _ADS_TCPheader {
    us	reserved;	// must be zero for now;
    u32 length;		//length in bytes
} ADS_TCPheader;


typedef struct _ADSpacket {
    ADS_TCPheader	adsHeader;
    AMSheader		amsHeader;
    char data[maxDataLen];
} ADSpacket;


*/

int _ADSwrite22(ADSInterface * di, void * buffer, int len) {
    int res;
    if (di==NULL) return -2;
    if (di->error) return -1;
    _ADSDump("I send: ", buffer, len);
    res=write(di->fd.wfd, buffer, len);
    if (res<0) di->error|=1;
    return res;
}    

void ranalyze(ADSConnection * dc) {
    ADSpacket * p=(ADSpacket *)(dc->msgIn);
    LOG2("ADS_TCP.header.reserved: %d\n", p->adsHeader.reserved);
    LOG2("ADS_TCPheader.length: %d\n", p->adsHeader.length);
    _ADSDumpAMSheader(&(p->amsHeader));
    uc * b=(uc*) p->data;
    ADSreadWriteRequest * rwrq;
    ADSreadWriteResponse * rwrs;
    ADSwriteRequest * wrq;
    ADSwriteResponse * wrs;
    ADSdeviceInfo * di;
//     p->data;
    
    memset(dc->msgOut,0,500);
    ADSpacket * pr=(ADSpacket *)(dc->msgOut);
    
    switch (p->amsHeader.commandId) {
	case cmdADSreadDevInfo:
	    di=(ADSdeviceInfo*) pr->data;
	    di->majorVersion=185;
	    di->minorVersion=0;
	    di->build=0;
	    di->name[0]='P';
	    di->name[1]='L';
	    di->name[2]='C';
	    di->name[3]=' ';
	    di->name[4]='C';
	    di->name[5]='O';
	    di->name[6]='U';
	    di->name[7]='P';
	    di->name[8]='L';
	    di->name[9]='E';
	    di->name[10]='R';
	    pr->amsHeader.dataLength= sizeof(ADSdeviceInfo);
	break;
	
	case cmdADSwrite:
	    wrq=(ADSwriteRequest*) p->data;
	    LOG2("Index Group:   %04x\n", wrq->indexGroup);
	    LOG2("Index Offset:  %d\n", wrq->indexOffset);
	    LOG2("Data length:  %d\n", wrq->length);
	    _ADSDump("Data: ", wrq->data, wrq->length);
	    wrs =(ADSwriteResponse *) (pr->data);
	    wrs->result=0;
	    pr->amsHeader.dataLength= 4;
	break;
	case cmdADSreadWrite:
	    rwrq =(ADSreadWriteRequest *) (p->data);
	    LOG2("Index Group:   %04x\n", rwrq->indexGroup);
	    LOG2("Index Offset:  %d\n", rwrq->indexOffset);
	    LOG2("Read data length:  %d\n", rwrq->readLength);
	    LOG2("Write data length: %d\n", rwrq->writeLength);
	    _ADSDump("WriteData: ", rwrq->data, rwrq->writeLength);    
	    
	    rwrs =(ADSreadWriteResponse *) (pr->data);
	    rwrs->result=0;
	    rwrs->length=rwrq->readLength;
	    
	    pr->amsHeader.dataLength= 8 + rwrq->readLength;
	    *(int*)(rwrs->data+0)=p->amsHeader.invokeId;
	    LOG2("Response data length: %d\n", pr->amsHeader.dataLength);
	    _ADSDump("Response ", rwrs->data, rwrq->writeLength);    
	break;
	default:
	    pr->amsHeader.dataLength= 4;
    }
//    ADSwriteRequest * rq=(ADSwriteRequest*) p->data;
    
//    pr->adsHeader.length=36;	// 76-32-8;
    pr->adsHeader.length=pr->amsHeader.dataLength+32;
    LOG2("Response data length: %d\n", pr->amsHeader.dataLength);
    LOG2("Response packet length: %d\n", pr->adsHeader.length);
    
    pr->amsHeader.targetId=p->amsHeader.sourceId;
    pr->amsHeader.targetPort=p->amsHeader.sourcePort;
    pr->amsHeader.sourceId=p->amsHeader.targetId;
    pr->amsHeader.sourcePort=p->amsHeader.targetPort;
    
    pr->amsHeader.commandId=p->amsHeader.commandId;
//    pr->amsHeader.commandId=0;
//    pr->amsHeader.stateFlags=p->amsHeader.stateFlags|1;
    
    pr->amsHeader.stateFlags=0x5;
    
//    pr->amsHeader.dataLength=4;
    pr->amsHeader.errorCode=0;
    pr->amsHeader.invokeId=p->amsHeader.invokeId;
    
    ADSwriteResponse * rp=(ADSwriteResponse*) pr->data;
    rp->result=0;
    LOG2("ADS_TCP.header.reserved: %d\n", pr->adsHeader.reserved);
    LOG3("ADS_TCPheader.length: %d total:%d\n", pr->adsHeader.length, sizeof(AMS_TCPheader)+
    sizeof( AMSheader)+4);
    _ADSDumpAMSheader(&(pr->amsHeader));
    _ADSwrite(dc);
};

typedef struct _portInfo {
    int fd;
}portInfo;


#define mymemcmp _ADSMemcmp
void *portServer(void *arg)
{
    portInfo * pi=(portInfo *) arg;
    LOG2("portMy fd is:%d\n", pi->fd);
    FLUSH;
    int waitCount = 0;
    ADSDebug=ADSDebugAll;
    int res;
    int pcount=0;
    _ADSOSserialType s;
    s.rfd=pi->fd;
    s.wfd=pi->fd;
    ADSInterface * di=ADSNewInterface(s,me,0x800,"IF");
    di->timeout=900000;
    ADSConnection * dc=ADSNewConnection(di,partner,800);
    while (waitCount < 1000) {
	dc->AnswLen=_ADSReadPacket(dc->iface, dc->msgIn);
	if (dc->AnswLen>0) {
	    res=dc->AnswLen;
	    LOG2("%d ", pcount);		
	    _ADSDump("packet", dc->msgIn, dc->AnswLen);
	    waitCount = 0;
	    ranalyze(dc);
		    
	    int r2=2*res;

	    pcount++;
	} else {
	    waitCount++;
	}    
    }
    LOG1("portserver: I closed my fd.\n");
    FLUSH;
    return NULL;
}


/*
    This waits in select for a file descriptor from accepter and starts a new child server
    with this file descriptor.
*/

int main(int argc, char **argv)
{
    if (argc<1) {
	printf("Usage: ADSserver port\n");
	printf("Example: ADSserver 48898\n");
	return -1;
    }
    
//    readCallBack=dummyRead;
//    writeCallBack=myWrite;
    
    int filedes[2], res, newfd;
    char * s,s2;
    s = argv[1];
    s2 = argv[2];
    LOG2("Main serv: %s\n", s);
    
    LOG2("Main serv: %s\n", s2);
    
    portInfo pi;

    fd_set FDS;
    pipe(filedes);
    pthread_attr_t attr;
    pthread_t ac, ps;
    accepter_info ai;
    ai.port = atol(s2);
    LOG2("Main serv: %d\n", ai.port);
    LOG2("Main serv: Accepter pipe fd: %d\n", ai.fd);
    ai.fd = filedes[1];
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    res=pthread_create(&ac, &attr, accepter, &ai /*&filedes[1] */ );
    do {
	FD_ZERO(&FDS);
	FD_SET(filedes[0], &FDS);

	LOG2("Main serv: about to select on %d\n",
	       filedes[0]);
	FLUSH;
	if (select(filedes[0] + 1, &FDS, NULL, &FDS, NULL) > 0) {
	    LOG1("Main serv: about to read\n");
	    res = read(filedes[0], &pi.fd, sizeof(pi.fd));
	    ps=0;		   
	    pthread_attr_init(&attr);
	    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	    res=pthread_create(&ps, &attr, portServer, &pi);
	    if(res) {
		LOG2("Main serv: create error:%s\n", strerror(res));		   
		close(newfd);
		usleep(100000);
	    }	   
	}
    }
    while (1);
    return 0;
}


/*
    Changes:
    
    14/07/2003 give a hint about usage
*/
