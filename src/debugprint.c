/*
 Implementation of BECKHOFF's ADS protocol.
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System
 May 2011.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of
 BECKHOFF Company. www.beckhoff.de

 Copyright (C) Gerhard Schiller (gerhard.schiller@gmail.com) 2013.

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

#ifdef DEBUGPRINT

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "AdsDEF.h"
#include "ads.h"
#include "debugprint.h"

// set to your desire to values defined in debugprint.h
int _ADSDebug = MSG_ERROR;
char *szApplication = NULL;

void _msgout(char *szFile, int iLineNo, int iType, char *szMsg)
{
	if(!(_ADSDebug & iType))
		return;
	if(szApplication)
		fprintf(logout, "%s\t", szApplication);
	fprintf(logout, "%s\t%d\t%s",_fileFromPath(szFile), iLineNo, szMsg);
}

void _msgcnt(int iType, char *szMsg)
{
	if(!(_ADSDebug & iType))
		return;

	fprintf(logout, szMsg);
}

char *_msgoutp(char *fmt, ...)
{
	va_list marker;
	static  char buffer[1024];

	va_start( marker, fmt );
	vsprintf( buffer, fmt, marker );
	va_end( marker );
	return buffer;
}

void _msgDumpPacket(char *szFile, int iLineNo, char *funcName, void *v, int len)
{
	if(!(_ADSDebug & MSG_PACKET_V))
		return;

	if(szApplication)
		fprintf(logout, "%s\t", szApplication);
	fprintf(logout, "%s\t%d\t%s\n",_fileFromPath(szFile), iLineNo, funcName);

	_doDump(v, len, true);
};

void _msgDumpAMSaddr(char *szFile, int iLineNo, int iType, PAmsAddr paddr)
{
	if(!(_ADSDebug & iType))
		return;

	if(szApplication)
		fprintf(logout, "%s\t", szApplication);
	fprintf(logout, "%s\t%d\tAMS Adress: %s Port: %d\n",
			_fileFromPath(szFile), iLineNo,
			_msgAmsNetID2string(&paddr->netId),	paddr->port);
}

// need this with and without DEBUGPRINT
char *_msgAmsNetID2string(PAmsNetID pNetid)
{
	static char buffer[64];
	sprintf(buffer, "%d.%d.%d.%d.%d.%d",
			pNetid->b[0], pNetid->b[1],
			pNetid->b[2], pNetid->b[3],
			pNetid->b[4], pNetid->b[5]);

	return buffer;
}



void _msgAnalyzeHeader(char *szFile, int iLineNo, int iType, AMSheader * h)
{
	if(!(_ADSDebug & iType))
		return;

	if(szFile){
		if(szApplication)
			fprintf(logout, "%s\t", szApplication);
		fprintf(logout, "%s\t%d\tAMSheader:\n", _fileFromPath(szFile), iLineNo);
	}
	else
		fprintf(logout, "AMSheader:\n");

	fprintf(logout, "\t\ttargetId:      %s\n", _msgAmsNetID2string(&(h->targetId)));
	fprintf(logout, "\t\ttargetPort:    %d\n", h->targetPort);
	fprintf(logout, "\t\tsourceId:      %s\n", _msgAmsNetID2string(&(h->sourceId)));
	fprintf(logout, "\t\tsourcePort:    %d\n", h->sourcePort);
	fprintf(logout, "\t\tcommandId:     %d %s\n", h->commandId, _ADSCommandName(h->commandId));
	fprintf(logout, "\t\tstateFlags:    %d %s\n", h->stateFlags, _ADSStateName(h->stateFlags));
	fprintf(logout, "\t\tdataLength:    %d\n", h->dataLength);
	fprintf(logout, "\t\terrorCode:     0x%x %s\n", h->errorCode, ADSerrorText(h->errorCode));
	fprintf(logout, "\t\tinvokeId:      %d\n", h->invokeId);
}

void _doDump(void *v, int len, bool doHex)
{
	unsigned char *b = (unsigned char *)v;
	int j;

	if (len > maxDataDump)
		len = maxDataDump;	// this will avoid to dump zillions of chars
	for (j = 0; j < len; j++) {
		if(doHex)
			fprintf(logout, "%02X ", b[j]);
		else
			fprintf(logout, "%3d ", b[j]);
		if(j % 16 == 15)
			fprintf(logout, "\n");
	}
	if(j % 16 != 0)
		fprintf(logout, "\n");
}

void _msgAnalyzePacket(char *szFile, int iLineNo, char* funcName, void *pv)
{
	ADSpacket *p;

	//Responses
	ADSdeviceInfo *di;
	ADSreadResponse *rresp;
	ADSwriteResponse *wresp;
	ADSstateResponse *sresp;
	ADSwriteControlResponse *wcresp;
	ADSaddDeviceNotificationResponse *adnresp;
	ADSdeleteNotificationResponse *ddnresp;
	ADSreadWriteResponse *rwresp;

	// Requests
	// device info request has no data
	ADSreadRequest *rreq;
	ADSwriteRequest *wreq;
	// state request has no data
	ADSwriteControlRequest *wcreq;
	ADSaddDeviceNotificationRequest *adnreq;
	ADSdeleteDeviceNotificationRequest *ddnreq;
	AdsDeviceNotification *dnr;
	ADSreadWriteRequest *rwreq;

	int i;

	if(!(_ADSDebug & MSG_PACKET))
		return;

	p = (ADSpacket *)pv;
	/*
	ads.c   532     ADSreadWriteBytes()
	Response: Dissection of AMSpacket
	*/
	if(szApplication)
		fprintf(logout, "%s\t", szApplication);
	fprintf(logout, "%s\t%d\t%s: Dissection of AMSpacket (%s)\n",
			_fileFromPath(szFile),
			iLineNo,
			funcName,
			((p->amsHeader.stateFlags & 1) == 0) ? "Request" : "Response");

	fprintf(logout, "\tADS_TCPheader:\n");
	fprintf(logout, "\t\treserved:      %d\n", p->adsHeader.reserved);
	fprintf(logout, "\t\tlength:        %d\n", p->adsHeader.length);

	fprintf(logout, "\t");
	_msgAnalyzeHeader(NULL, 0, MSG_PACKET, &(p->amsHeader));

	fprintf(logout, "\tData:\n");
	if ((p->amsHeader.stateFlags & 1) == 0) {
		// REQUEST
		switch (p->amsHeader.commandId) {
			case cmdADSreadDevInfo:
				break;

			case cmdADSread:
				rreq = (ADSreadRequest *) (pv + 38);
				fprintf(logout, "\t\tindexGroup:    0x%x\n", rreq->indexGroup);
				fprintf(logout, "\t\tindexOffset:   %d\n", rreq->indexOffset);
				fprintf(logout, "\t\tread length:   %d\n", rreq->length);
				break;

			case cmdADSwrite:
				wreq = (ADSwriteRequest *) (pv + 38);
				fprintf(logout, "\t\tindexGroup:    0x%x\n", wreq->indexGroup);
				fprintf(logout, "\t\tindexOffset:   %d\n", wreq->indexOffset);
				fprintf(logout, "\t\tread length:   %d\n", wreq->length);
				if(wreq->length > 0){
					fprintf(logout, "\t\tData:          ");
					_doDump((void *)wreq->data, wreq->length, true);
				}
				break;

			case cmdADSwriteControl:
				wcreq = (ADSwriteControlRequest *) (pv + 38);
				fprintf(logout, "\t\tADSstate:      %d\n", wcreq->ADSstate);
				fprintf(logout, "\t\tdevState:      %d\n", wcreq->devState);
				fprintf(logout, "\t\twrite length:  %d\n", wcreq->length);
				if(wcreq->length > 0){
					fprintf(logout, "\t\tData:          ");
					_doDump((void *)wcreq->data, wcreq->length, true);
				}
				break;

			case cmdADSaddDeviceNotification:
				adnreq = (ADSaddDeviceNotificationRequest *) (pv + 38);
				fprintf(logout, "\t\tindexGroup:    0x%x\n", adnreq->indexGroup);
				fprintf(logout, "\t\tindexOffset:   %d\n", adnreq->indexOffset);
				fprintf(logout, "\t\tread length:   %d\n", adnreq->length);
				fprintf(logout, "\t\ttransm. mode:  %d\n", adnreq->transmissionMode);
				fprintf(logout, "\t\tmaxDelay:      %d\n", adnreq->maxDelay);
				fprintf(logout, "\t\tcycleTime:     %d\n", adnreq->cycleTime);
				fprintf(logout, "\t\treserved:      ");
				_doDump((void *)adnreq->reserved, 16, true);
				break;

			case cmdADSdeleteDeviceNotification:
				ddnreq = (ADSdeleteDeviceNotificationRequest *) (pv + 38);
				fprintf(logout, "\t\thNotification: %d\n", ddnreq->hNotification);
				break;

			case cmdADSdevNotify:
				dnr = (AdsDeviceNotification *) (pv + 38);
				fprintf(logout, "\t\tlength:        %d\n", dnr->nLength);
				fprintf(logout, "\t\tstamps:        %d\n", dnr->nStamps);
				fprintf(logout, "\t\tData:          ");
				_doDump((void *)dnr + 8, dnr->nLength, true);
							// or dnr->nLenght - 8 ?
							// anyway, there are more bytes in "data" then needed !!!
			break;

			case cmdADSreadWrite:
				rwreq = (ADSreadWriteRequest *) (pv + 38);
				fprintf(logout, "\t\tindexGroup:    0x%x\n", rwreq->indexGroup);
				fprintf(logout, "\t\tindexOffset:   %d\n", rwreq->indexOffset);
				fprintf(logout, "\t\tread length:   %d\n", rwreq->readLength);
				fprintf(logout, "\t\twrite length:  %d\n", rwreq->writeLength);
				if(rwreq->writeLength > 0){
					fprintf(logout, "\t\tData:          ");
					_doDump((void *)rwreq->data, rwreq->writeLength, true);
// 					_doDump((void *)rwreq->data, rwreq->readLength, TRUE);
				}
				break;
		}
	}
	else{
		// RESPONSE
		switch (p->amsHeader.commandId) {
			case cmdADSreadDevInfo:
				di = (ADSdeviceInfo *) (pv + 38);
				fprintf(logout, "\t\terrorCode:     0x%x %s\n", di->result, ADSerrorText(di->result));
				fprintf(logout, "\t\tversion:       %d.%d\n", di->Version.version,di->Version.revision);
				fprintf(logout, "\t\tbuild:         %d\n", di->Version.build);
				fprintf(logout, "\t\tdeviceName:    ");
				for (i = 0; i < 15; i++) {
					fprintf(logout, "%c", di->name[i]);
				}
				fprintf(logout, "\n");
				break;

			case cmdADSread:
				rresp = (ADSreadResponse *) (pv + 38);
				fprintf(logout, "\t\terrorCode:     0x%x %s\n", rresp->result, ADSerrorText(rresp->result));
				fprintf(logout, "\t\tresp. length:  %d\n", rresp->length);
				if(rresp->length > 0){
					fprintf(logout, "\t\tData:          ");
					_doDump((void *)rresp->data, rresp->length, true);
				}
				break;

			case cmdADSwrite:
				wresp = (ADSwriteResponse *) (pv + 38);
				fprintf(logout, "\t\terrorCode:     0x%x %s\n", wresp->result, ADSerrorText(wresp->result));
				break;

			case cmdADSreadState:
				sresp = (ADSstateResponse *) (pv + 38);
				fprintf(logout, "\t\terrorCode:     0x%x %s\n", sresp->result, ADSerrorText(sresp->result));
				fprintf(logout, "\t\tADS state:     %d\n", sresp->ADSstate);
				fprintf(logout, "\t\tdevice state:  %d\n", sresp->devState);
				break;

			case cmdADSwriteControl:
				wcresp = (ADSwriteControlResponse *) (pv + 38);
				fprintf(logout, "\t\terrorCode:     0x%x %s\n", wcresp->result, ADSerrorText(wcresp->result));
				break;

			case cmdADSaddDeviceNotification:
				adnresp = (ADSaddDeviceNotificationResponse *) (pv + 38);
				fprintf(logout, "\t\terrorCode:     0x%x %s\n", adnresp->result, ADSerrorText(adnresp->result));
				fprintf(logout, "\t\tHandle:        %d\n", adnresp->notificationHandle);
				break;

			case cmdADSdeleteDeviceNotification:
				ddnresp = (ADSdeleteNotificationResponse *) (pv + 38);
				fprintf(logout, "\t\terrorCode:     0x%x %s\n", ddnresp->result, ADSerrorText(ddnresp->result));
				break;

			case cmdADSreadWrite:
				rwresp = (ADSreadWriteResponse *) (pv + 38);
				fprintf(logout, "\t\terrorCode:     0x%x %s\n", rwresp->result, ADSerrorText(rwresp->result));
				fprintf(logout, "\t\tdata length:   %d\n", rwresp->length);
				if(rwresp->length > 0){
					fprintf(logout, "\t\tData:          ");
// 					_doDump(p + 44, rwresp->length, TRUE);
					_doDump(rwresp->data, rwresp->length, true);
				}
				break;

		}
	}
}

char *_ADSCommandName(int c)
{
	switch (c) {
		case 0:
			return ("cmdADSinvalid");
		case 1:
			return ("cmdADSreadDevInfo");
		case 2:
			return ("cmdADSread");
		case 3:
			return ("cmdADSwrite");
		case 4:
			return ("cmdADSreadState");
		case 5:
			return ("cmdADSwriteControl");
		case 6:
			return ("cmdADSaddDevNotify");
		case 7:
			return ("cmdADSdelDevNotify");
		case 8:
			return ("cmdADSdevNotify");
		case 9:
			return ("cmdADSreadWrite");
		default:
			return ("unknown !");
	}
}

char *_ADSStateName(int c)
{
	if((c & 04) == 0)
		return ("NOT an ADS Command");

	switch (c & 07) {
		case 04:
			return ("stateADSrequest");
		case 05:
			return ("stateADSresponse");
		default:
			return ("unknown !");
	}
}

char *_fileFromPath(char * path)
{
	char *cp;

	cp = path + strlen(path);
	while(*cp != '/'&& cp >= path)
		cp--;
	cp++;
	return cp;
}

#endif	//DEBUGPRINT
