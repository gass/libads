/*
 Implementation of BECKHOFF's ADS protocol.
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System
 May 2011.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of
 BECKHOFF Company. www.beckhoff.de

 Copyright (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2003.
 Copyright (C) Luis Matos (gass@otiliamatos.ath.cx) 2009,2013

 This file is part of libads.  
 Libads is free software: you can redistribute it and/or modify
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <byteswap.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

#define ThisModule "ADSserver : "

#include "ads.h"
#include "ads_io.h" //TODO: call this from ads.h
#include "debugprint.h"
#include "accepter.h"

AMSNetID me;

AMSNetID partner;

/*
    many bytes. hopefully enough to serve any read request.
*/
unsigned char dummyRes[] =
	{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
};

void ranalyze(ADSConnection * dc)
{
	ADSpacket *p = (ADSpacket *) (dc->msgIn);
	ads_debug(ADSDebug, "ADS_TCP.header.reserved: %d\n",
		  p->adsHeader.reserved);
	ads_debug(ADSDebug, "ADS_TCPheader.length: %d\n", p->adsHeader.length);
	_msgAnalyzeHeader(__FILE__, __LINE__, MSG_PACKET, &(p->amsHeader));
	ADSreadRequest *rrq;
	ADSreadResponse *rrs;
	ADSreadWriteRequest *rwrq;
	ADSreadWriteResponse *rwrs;
	ADSwriteRequest *wrq;
	ADSwriteResponse *wrs;
	ADSdeviceInfo *di;
	ADSstateResponse *asr;
	int nErr;
	
	memset(dc->msgOut, 0, 500);
	ADSpacket *pr = (ADSpacket *) (dc->msgOut);

	switch (p->amsHeader.commandId) {
	case cmdADSreadDevInfo:
		di = (ADSdeviceInfo *) pr->data;
		di->Version.version = 185;
		di->Version.revision = 0;
		di->Version.build = 0;
		di->name[0] = 'P';
		di->name[1] = 'L';
		di->name[2] = 'C';
		di->name[3] = ' ';
		di->name[4] = 'C';
		di->name[5] = 'O';
		di->name[6] = 'U';
		di->name[7] = 'P';
		di->name[8] = 'L';
		di->name[9] = 'E';
		di->name[10] = 'R';
		pr->amsHeader.dataLength = sizeof(ADSdeviceInfo);
		break;
	case cmdADSreadState:
		asr = (ADSstateResponse *) pr->data;
		asr->result = 0x0000;
		asr->ADSstate = ADSSTATE_RUN;
		asr->devState = 0xA;
		pr->amsHeader.dataLength = sizeof(ADSstateResponse);
		break;

	case cmdADSwrite:
		wrq = (ADSwriteRequest *) p->data;
		ads_debug(ADSDebug, "Index Group:   %04x\n", wrq->indexGroup);
		ads_debug(ADSDebug, "Index Offset:  %d\n", wrq->indexOffset);
		ads_debug(ADSDebug, "Data length:  %d\n", wrq->length);
		//_ADSDump("Data: ", wrq->data, wrq->length);
		wrs = (ADSwriteResponse *) (pr->data);
		wrs->result = 0;
		pr->amsHeader.dataLength = 4;
		break;
	case cmdADSreadWrite:
		rwrq = (ADSreadWriteRequest *) (p->data);
		ads_debug(ADSDebug, "Index Group:   %04x\n", rwrq->indexGroup);
		ads_debug(ADSDebug, "Index Offset:  %d\n", rwrq->indexOffset);
		ads_debug(ADSDebug, "Read data length:  %d\n",
			  rwrq->readLength);
		ads_debug(ADSDebug, "Write data length: %d\n",
			  rwrq->writeLength);
		//_ADSDump("WriteData: ", rwrq->data, rwrq->writeLength);

		rwrs = (ADSreadWriteResponse *) (pr->data);
		rwrs->result = 0;
		rwrs->length = rwrq->readLength;

		pr->amsHeader.dataLength = 8 + rwrq->readLength;
		*(int *)(rwrs->data + 0) = p->amsHeader.invokeId;
		ads_debug(ADSDebug, "Response data length: %d\n",
			  pr->amsHeader.dataLength);
		//_ADSDump("Response ", rwrs->data, rwrq->writeLength);
		break;

	case cmdADSread:
		rrq = (ADSreadRequest *) (p->data);
		rrs = (ADSreadResponse *) (pr->data);
		rrs->result = 0;
		rrs->length = rrq->length;
		pr->amsHeader.dataLength = 8 + rrq->length;
		*(int *)(rrs->data + 0) = p->amsHeader.invokeId;
		memcpy(rrs->data + 4, dummyRes + rrq->indexOffset, sizeof(dummyRes));
		break;
		
		
	default:
		printf("Unhandeled command: %s\n", _ADSCommandName(p->amsHeader.commandId));
		pr->amsHeader.dataLength = 4;
	}
	pr->adsHeader.length = pr->amsHeader.dataLength + 32;
	ads_debug(ADSDebug, "Response data length: %d\n",
		  pr->amsHeader.dataLength);
	ads_debug(ADSDebug, "Response packet length: %d\n",
		  pr->adsHeader.length);

	pr->amsHeader.targetId = p->amsHeader.sourceId;
	pr->amsHeader.targetPort = p->amsHeader.sourcePort;
	pr->amsHeader.sourceId = p->amsHeader.targetId;
	pr->amsHeader.sourcePort = p->amsHeader.targetPort;

	pr->amsHeader.commandId = p->amsHeader.commandId;
	pr->amsHeader.stateFlags = 0x5;
	pr->amsHeader.errorCode = 0;
	pr->amsHeader.invokeId = p->amsHeader.invokeId;

	ADSwriteResponse *rp = (ADSwriteResponse *) pr->data;
	rp->result = 0;
	ads_debug(ADSDebug, "ADS_TCP.header.reserved: %d\n",
		  pr->adsHeader.reserved);
	ads_debug(ADSDebug, "ADS_TCPheader.length: %d total:%d\n",
		  pr->adsHeader.length,
		  sizeof(AMS_TCPheader) + sizeof(AMSheader) + 4);
	_msgAnalyzeHeader(__FILE__, __LINE__, MSG_PACKET, &(pr->amsHeader));
	_ADSWritePacket(dc->iface, pr, &nErr);
};

void *portServer(void *arg)
{
	int *fd = (int *)arg;
	int nErr;
	ads_debug(ADSDebug, "portMy fd is:%d\n", fd);
	int waitCount = 0;
	int pcount = 0;
	ADSInterface *di = _ADSNewInterface(*fd, me, AMSPORT_R0_PLC_RTS1, "IF");
	di->timeout = 900000;
	ADSConnection *dc = _ADSNewConnection(di, partner, AMSPORT_R0_PLC_RTS1);
	while (waitCount < 1000) {
		dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn, &nErr);
		if (dc->AnswLen > 0) {
			ads_debug(ADSDebug, "%d ", pcount);
			//_ADSDump("packet", dc->msgIn, dc->AnswLen);
			waitCount = 0;
			ranalyze(dc);

			pcount++;
		} else {
			waitCount++;
		}
	}
	ads_debug(ADSDebug, "portserver: I closed my fd.\n");
	;
	return NULL;
}

/*
    This waits in select for a file descriptor from accepter and starts a new child server
    with this file descriptor.
*/

int main(int argc, char **argv)
{
	if (argc <= 1) {
		printf("Usage: ADSserver port\n");
		printf("Example: ADSserver 48898\n");
		return -1;
	}

	ADSGetLocalAMSId(&me);
	ADSGetLocalAMSId(&partner);

	int filedes[2], res, newfd = 0;
	char *s;
	s = argv[1];
	ads_debug(ADSDebug, "Main serv: %s\n", s);

	int fd;

	fd_set FDS;
	pipe(filedes);
	pthread_attr_t attr;
	pthread_t ac, ps;
	accepter_info ai;
	ai.port = atol(s);
	ads_debug(ADSDebug, "Main serv: %d\n", ai.port);
	ads_debug(ADSDebug, "Main serv: Accepter pipe fd: %d\n", ai.fd);
	ai.fd = filedes[1];
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	res = pthread_create(&ac, &attr, accepter, &ai /*&filedes[1] */ );
	do {
		FD_ZERO(&FDS);
		FD_SET(filedes[0], &FDS);

		ads_debug(ADSDebug, "Main serv: about to select on %d\n",
			  filedes[0]);
		;
		if (select(filedes[0] + 1, &FDS, NULL, &FDS, NULL) > 0) {
			ads_debug(ADSDebug, "Main serv: about to read\n");
			res = read(filedes[0], &fd, sizeof(fd));
			ps = 0;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,
						    PTHREAD_CREATE_DETACHED);
			res = pthread_create(&ps, &attr, portServer, &fd);
			if (res) {
				ads_debug(ADSDebug,
					  "Main serv: create error:%s\n",
					  strerror(res));
				close(newfd);
				usleep(100000);
			}
		}
	}
	while (1);
	return 0;
}
