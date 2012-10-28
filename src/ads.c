/*
 Implementation of BECKHOFF's ADS protocol. 
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System Nov 2002.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of BECKHOFF 
 Company. www.beckhoff.de

 (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2003.
 Copyright (C) Luis Matos (gass@otiliamatos.ath.cx) 2012.

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

#ifdef __linux__
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>

#include "ads.h"
#include "AdsDEF.h"

char *ADSCommandName(int c)
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

/*
 * Debug function.
 * sends some output to stdout if you define DEBUG
 */
void ads_debug(int type, const char *fmt, ...)
{
	va_list args;
	char tmp_str[1000] = { 0, };
	if (ADSDebug & type) {
		va_start(args, fmt);
		vsnprintf(tmp_str, 1000, fmt, args);
		printf("LOG: %s\n", tmp_str);
		va_end(args);
	}
}

void _ADSDumpAMSNetId(AMSNetID * id)
{
	_ADSDump("AMSNetID: ", (uc *) id, sizeof(AMSNetID));
}

void _ADSDumpAMSheader(AMSheader * h)
{
	if (!ADSDebug)
		return;
	printf("DumpAMS header:\n");
	printf("targetId");
	_ADSDumpAMSNetId(&(h->targetId));
	printf("targetPort: %d\n", h->targetPort);
	printf("sourceId");
	_ADSDumpAMSNetId(&(h->sourceId));
	printf("sourcePort: %d\n", h->sourcePort);
	printf("commandId: %d=%s\n", h->commandId,
	       ADSCommandName(h->commandId));
//    printf("stateFlags: %d=%s\n", h->stateFlags,ADSstateFlagsName(h->stateFlags));
	printf("stateFlags: %d\n", h->stateFlags);
	printf("dataLength: %d\n", h->dataLength);
	printf("errorCode:  %04x %s\n", h->errorCode,
	       ADSerrorText(h->errorCode));
	printf("invokeId:   %d\n", h->invokeId);
}

char *ADSerrorText(int err)
{
	switch (err) {
	case 0:
		return "no error";
	case 0x0001:
		return "Internal error";
	case 0x0002:
		return "No Rtime";
	case 0x0003:
		return "Allocation locked memory error";
	case 0x0004:
		return "Insert mailbox error";
	case 0x0005:
		return "Wrong receive HMSG";
	case 0x0006:
		return "target port not found";
	case 0x0007:
		return "target machine not found";
	case 0x0008:
		return "unknown command ID";
	case 0x0009:
		return "Bad task ID";
	case 0x000A:
		return "No IO";
	case 0x000B:
		return "unknown AMS command";
	case 0x000C:
		return "Win 32 error";
	case 0x000D:
		return "port not connected";
	case 0x000E:
		return "invalid AMS length";
	case 0x000F:
		return "invalid AMS Net ID";
	case 0x0010:
		return "Low installation level";
	case 0x0011:
		return "No debug available";
	case 0x0012:
		return "Port disabled";
	case 0x0013:
		return "Port already connected";
	case 0x0014:
		return "AMS Sync Win 32 error";
	case 0x0015:
		return "AMS Sync timeout";
	case 0x0016:
		return "AMS Sync AMS error";
	case 0x0017:
		return "AMS Sync no index map";
	case 0x0018:
		return "Invalid AMS port";
	case 0x0019:
		return "No memory";
	case 0x001A:
		return "TCP send error";
	case 0x001B:
		return "Host unreachable";

	case 0x0500:
		return "Router: no locked memory";
	case 0x0502:
		return "Router: mailbox full";

	case 0x0700:
		return "error class <device error>??";
	case 0x0701:
		return "service is not supported by server";
	case 0x0702:
		return "invalid index group";
	case 0x0703:
		return "invalid index offset";
	case 0x0704:
		return "reading/writing not permitted";
	case 0x0705:
		return "parameter size not correct";
	case 0x0706:
		return "invalid parameter value(s)";
	case 0x0707:
		return "device is not in a ready state";
	case 0x0708:
		return "device is busy";
	case 0x0709:
		return "invalid context. (must be in Windows ?)";
	case 0x070A:
		return "out of memory";

	case 0x070B:
		return "invalid parameter value(s)";
	case 0x070C:
		return "not found (files,...)";
	case 0x070D:
		return "syntax error in command or file";
	case 0x070E:
		return "objects do not match";
	case 0x070F:
		return "object already exists";
	case 0x0710:
		return "symbol not found";
	case 0x0711:
		return "symbol version invalid";

	case 0x0712:
		return "server is in invalid state";
	case 0x0713:
		return "AdsTransMode not supported";
	case 0x0714:
		return "Notification handle is invalid";
	case 0x0715:
		return "Notification client not registered";
	case 0x0716:
		return "No more notification handles";
	case 0x0717:
		return "Size for watch too big";
	case 0x0718:
		return "device not initialized";
	case 0x0719:
		return "device has a timeout";

	case 0x0740:
		return "error class <client error>??";
	case 0x0741:
		return "invalid parameter at service";
	case 0x0742:
		return "polling list is empty";
	case 0x0743:
		return "var connection already in use";
	case 0x0744:
		return "invoke ID in use";
	case 0x0745:
		return "timeout elapsed";
	case 0x0746:
		return "error in win32 subsystem";
	case 0x0748:
		return "ads port not opened";

	case 0x0750:
		return "internal error in ads sync";
	case 0x0751:
		return "Hash table overflow";
	case 0x0752:
		return "key not found in hash";
	case 0x0753:
		return "no more symbols in cache";
	default:
		return "Unknown error code";
	}
}

void analyze(uc * p1)
{
	ADSreadResponse *rr;
	ADSdeviceInfo *di;
	ADSstateResponse *sr;
	ADSaddDeviceNotificationResponse *adnr;
	int i;
	ADSpacket *p = (ADSpacket *) p1;
	ads_debug(ADSDebugAnalyze, "ADS_TCP.header.reserved: %d\n",
		  p->adsHeader.reserved);
	ads_debug(ADSDebugAnalyze, "ADS_TCPheader.length: %d\n",
		  p->adsHeader.length);
	_ADSDumpAMSheader(&(p->amsHeader));

	if ((p->amsHeader.stateFlags && 1) == 0) {
		switch (p->amsHeader.commandId) {
		}
	} else
		switch (p->amsHeader.commandId) {
		case cmdADSreadDevInfo:
			di = (ADSdeviceInfo *) (p1 + 38);
			ads_debug(ADSDebugAnalyze, "Error: %d %s\n",
				  di->ADSerror, ADSerrorText(di->ADSerror));
			ads_debug(ADSDebugAnalyze, "Version: %d.%d\n",
				  di->Version.version, di->Version.revision);
			ads_debug(ADSDebugAnalyze, "Build: %d\n",
				  di->Version.build);
			ads_debug(ADSDebugAnalyze, "Device name: ");
			for (i = 0; i < 15; i++) {
				ads_debug(ADSDebugAnalyze, "%c", di->name[i]);
			}
			ads_debug(ADSDebugAnalyze, "\n");
			break;

		case cmdADSread:
			rr = (ADSreadResponse *) (p1 + 38);
			ads_debug(ADSDebugAnalyze, "Result: %d %s\n",
				  rr->result, ADSerrorText(rr->result));
			ads_debug(ADSDebugAnalyze, "Data length: %d\n",
				  rr->length);
			_ADSDump("Data: ", (uc *) p1 + 44, rr->length);
			break;

		case cmdADSwrite:
			rr = (ADSreadResponse *) (p1 + 38);
			ads_debug(ADSDebugAnalyze, "Result: %d %s\n",
				  rr->result, ADSerrorText(rr->result));
			ads_debug(ADSDebugAnalyze, "Data length: %d\n",
				  rr->length);
			break;

		case cmdADSreadState:
			sr = (ADSstateResponse *) (p1 + 38);
			ads_debug(ADSDebugAnalyze, "Result: %d %s\n",
				  sr->result, ADSerrorText(sr->result));
			ads_debug(ADSDebugAnalyze, "ADS state: %d\n",
				  sr->ADSstate);
			ads_debug(ADSDebugAnalyze, "Device state: %d\n",
				  sr->devState);
			break;

		case cmdADSaddDeviceNotification:
			adnr = (ADSaddDeviceNotificationResponse *) (p1 + 38);
			ads_debug(ADSDebugAnalyze, "Result: %d %s\n",
				  adnr->result, ADSerrorText(adnr->result));
			ads_debug(ADSDebugAnalyze, "Handle: %d\n",
				  adnr->notificationHandle);
			break;

		}
}

ADSInterface *ADSNewInterface(_ADSOSserialType nfd, AMSNetID me, int port,
			      char *nname)
{
	ADSInterface *di = (ADSInterface *) calloc(1, sizeof(ADSInterface));
	if (di) {
		di->name = nname;
		di->fd = nfd;
		di->timeout = 1000000;
		di->me = me;
		di->AMSport = port;
	}
	return di;
};

/**
 * Frees the allocated space for ADSInterface
 */
int freeADSInterface(ADSInterface * di)
{
	free(di);
	return 0;
}

/**
 * Frees the allocated space for ADSConnection
 */
int freeADSConnection(ADSConnection * dc)
{
	freeADSInterface(dc->iface);
	free(dc);
	return 0;
}

/** 
    This will setup a new connection structure using an initialized
    ADSInterface.
*/
ADSConnection *ADSNewConnection(ADSInterface * di, AMSNetID partner, int port)
{
	ADSConnection *dc = (ADSConnection *) calloc(1, sizeof(ADSConnection));
	if (dc) {
//      di->name=nname;
		dc->iface = di;
		dc->partner = partner;
		dc->AMSport = port;
	}
	return dc;
}

/**
    Hex dump:
*/
void _ADSDump(char *name, void *v, int len)
{
	if (!ADSDebug)
		return;
	uc *b = (uc *) v;
	int j;
	printf("%s: ", name);
	if (len > maxDataLen)
		len = maxDataLen;	// this will avoid to dump zillions of chars
	for (j = 0; j < len; j++) {
		printf("%02X,", b[j]);
	}
	printf("\n");
};

#ifdef __linux__
int _ADSReadOne(ADSInterface * di, uc * b)
{
	fd_set FDS;
	struct timeval t;
	FD_ZERO(&FDS);
	FD_SET(di->fd.rfd, &FDS);

	t.tv_sec = di->timeout / 1000000;
	t.tv_usec = di->timeout % 1000000;
	if (select(di->fd.rfd + 1, &FDS, NULL, NULL, &t) <= 0) {
		ads_debug(ADSDebugPrintErrors, "timeout in readOne.\n");
		return (0);
	} else {
		return read(di->fd.rfd, b, 1);
	}
};
#endif

#ifdef WIN32
int _ADSReadOne(ADSInterface * di, uc * b)
{
	unsigned long i;
	ReadFile(di->fd.rfd, b, maxDataLen, &i, NULL);
//  _ADSDump(ADSDebugByte, "got",buffer,i);
	return i;
}
#endif

/**
    Read one complete packet. First bytes contain length information.
*/
int _ADSReadPacket(ADSInterface * di, uc * b)
{
	AMS_TCPheader *h = (AMS_TCPheader *) b;
	int i, res = 0;
	res += _ADSReadOne(di, b);
	res += _ADSReadOne(di, b + res);
	res += _ADSReadOne(di, b + res);
	res += _ADSReadOne(di, b + res);
	res += _ADSReadOne(di, b + res);
	res += _ADSReadOne(di, b + res);
	if ((res > 0) && (res < 6))
		res += _ADSReadOne(di, b + res);
	if ((res > 0) && (res < 6))
		res += _ADSReadOne(di, b + res);
	if (res < 6) {
		if (ADSDebug & ADSDebugByte) {
			ads_debug(ADSDebugByte, "res %d ", res);
			_ADSDump("_ADSReadIBHpacket: short packet", b, res);
		}
		return (0);	// short packet
	}
/*
    This may be shorter, but we would hang on malformed packets.
    res+=read(fd, b+res,b[2]+5 ); //length +8 -3 already read    
    So do it byte by byte.
    Somebody might have a better solution using non-blocking something.
*/
	ads_debug(ADSDebugByte, "h.length %d\n", h->length);
	while (res < h->length + 6) {
		i = _ADSReadOne(di, b + res);	//length +8 -3 already read
		if (i < 1)
			return (0);
		res += i;
	}
	ads_debug(ADSDebugByte, "readADSpacket: %d bytes read, %d needed\n",
		  res, h->length + 6);
	return (res);
};

void _ADSsetupHeader(ADSConnection * dc, AMSheader * h)
{
	h->targetId = dc->partner;
	h->targetPort = dc->AMSport;
	h->sourceId = dc->iface->me;
	h->sourcePort = dc->iface->AMSport;
	h->stateFlags = 0x0004;	/* command and tcp connection TODO: this shpuld be configured */
	h->errorCode = 0;
	h->invokeId = ++dc->invokeId;	//user-defined 32-bit field. Usually it is used to identify
};

int _ADSwrite2(ADSInterface * di, void *buffer, int len)
{
	int res;
	if (di == NULL)
		return -2;
	if (di->error)
		return -1;
	if (ADSDebug & ADSDebugByte)
		_ADSDump("I send: ", buffer, len);
	res = write(di->fd.wfd, buffer, len);
	if (res < 0)
		di->error |= 1;
	return res;
}

int _ADSwrite(ADSConnection * dc)
{
	ADSpacket *p1 = (ADSpacket *) dc->msgOut;
	return _ADSwrite2(dc->iface, dc->msgOut, p1->adsHeader.length + 6);
}

int
ADSreadBytes(ADSConnection * dc, int indexGroup, int offset, int length,
	     void *buffer)
{
	AMSheader *h1;
	ADSpacket *p2;
	ADSreadResponse *rr;
	ADSreadRequest *rq;
	ADSpacket *p1 = (ADSpacket *) dc->msgOut;

	h1 = &(p1->amsHeader);
	_ADSsetupHeader(dc, h1);
	h1->commandId = cmdADSread;
	h1->dataLength = sizeof(ADSreadRequest);
	if ((ADSDebug & ADSDebugPacket) != 0) {
		_ADSDumpAMSheader(h1);
	}

	p1->adsHeader.length = h1->dataLength + 32;
	p1->adsHeader.reserved = 0;

	rq = (ADSreadRequest *) & p1->data;
	rq->indexGroup = indexGroup;
	rq->indexOffset = offset;
	rq->length = length;

	ads_debug(ADSDebugPacket, "Index Group:   %x\n", rq->indexGroup);
	ads_debug(ADSDebugPacket, "Index Offset:  %d\n", rq->indexOffset);
	ads_debug(ADSDebugPacket, "Data length:  %d\n", rq->length);

	_ADSwrite(dc);
	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn);
	if (dc->AnswLen > 0) {
//      res=dc->AnswLen;
		if ((ADSDebug & ADSDebugPacket) != 0) {
			_ADSDump("packet", dc->msgIn, dc->AnswLen);
		}
		if ((ADSDebug & ADSDebugAnalyze) != 0) {
			analyze(dc->msgIn);
		}
		p2 = (ADSpacket *) dc->msgIn;

		if (p2->amsHeader.commandId == cmdADSread) {
			rr = (ADSreadResponse *) (dc->msgIn + 38);
			if (rr->result != 0)
				return rr->result;
//          ads_debug("Here we are\n");
//          _ADSDump("Data: ", (uc*) (dc->msgIn+44), rr->length);
			dc->dataPointer = dc->msgIn + 46;
			dc->AnswLen = rr->length;
			if (buffer != NULL) {
				memcpy(buffer, dc->dataPointer, rr->length);
			}
		}
		return 0;
	}
	return -1;
}

int
ADSreadWriteBytes(ADSConnection * dc,
		  int indexGroup,
		  int offset,
		  int readLength,
		  void *readBuffer, int writeLength, void *writeBuffer)
{

	AMSheader *h1;
	AMS_TCPheader *h2;
	ADSreadWriteRequest *rq;
	ADSreadWriteResponse *rr;
	ADSpacket *p1 = (ADSpacket *) dc->msgOut;
	ADSpacket *p2;
	h1 = &(p1->amsHeader);
	h2 = &(p1->adsHeader);
	_ADSsetupHeader(dc, h1);
	h1->commandId = cmdADSreadWrite;
	h1->dataLength = sizeof(ADSreadWriteRequest) - maxDataLen + writeLength;

	_ADSDumpAMSheader(h1);

	h2->length = h1->dataLength + sizeof(AMSheader);
	h2->reserved = 0;
	ads_debug(ADSDebug, "lenght: %d\n", h2->length);
	rq = (ADSreadWriteRequest *) & p1->data;
	rq->indexGroup = indexGroup;
	rq->indexOffset = offset;
	rq->writeLength = writeLength;
	rq->readLength = readLength;
	if (writeBuffer != NULL) {
		memcpy(rq->data, writeBuffer, writeLength);
	}
	_ADSDump("packet", dc->msgOut, p1->adsHeader.length + 6);

	ads_debug(ADSDebugPacket, "Index Group:   %x\n", rq->indexGroup);
	ads_debug(ADSDebugPacket, "Index Offset:  %d\n", rq->indexOffset);
	ads_debug(ADSDebugPacket, "Data length:  %d\n", rq->writeLength);
	_ADSwrite(dc);
	/*Reads the answer */
	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn);

	/* Analises the received packet */
	if (dc->AnswLen > 0) {
		p2 = (ADSpacket *) dc->msgIn;
		if (p2->amsHeader.commandId == cmdADSreadWrite) {

			_ADSDump(" readpacket", dc->msgIn, dc->AnswLen);
			rr = (ADSreadWriteResponse *) (dc->msgIn + 38);
			printf("this  is it %d\n", *rr->data);
			printf("this  is it %d\n", *(int *)readBuffer);
			memcpy(readBuffer, rr->data, readLength);
			printf("this  is it %d\n", *(int *)readBuffer);
		}
	}

	else {			/* if there is an error */
		if ((ADSDebug & ADSDebugPacket) != 0)
			_ADSDump("packet", dc->msgIn, dc->AnswLen);

		if ((ADSDebug & ADSDebugAnalyze) != 0)
			analyze(dc->msgIn);
	}
	return 0;
}

int ADSreadDeviceInfo(ADSConnection * dc, char *pDevName, PAdsVersion pVersion)
{
	AMSheader *h1;
	ADSpacket *p1 = (ADSpacket *) dc->msgOut;
	ADSpacket *p2;
	ADSdeviceInfo *DeviceInfo;
	h1 = &(p1->amsHeader);
	_ADSsetupHeader(dc, h1);
	h1->commandId = cmdADSreadDevInfo;
	h1->dataLength = 0;
	_ADSDumpAMSheader(h1);

	p1->adsHeader.length = h1->dataLength + AMSHeaderSIZE;
	p1->adsHeader.reserved = 0;

	_ADSwrite(dc);
	/*Reads the answer */
	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn);

	/* Analises the received packet */
	if ((ADSDebug & ADSDebugAnalyze) != 0)
		analyze(dc->msgIn);
	if ((ADSDebug & ADSDebugPacket) != 0)
		_ADSDump("packet ", dc->msgIn, dc->AnswLen);

	if (dc->AnswLen > 0) {
		p2 = (ADSpacket *) dc->msgIn;
		if (p2->amsHeader.commandId == cmdADSreadDevInfo) {
			DeviceInfo =
			    (ADSdeviceInfo *) (dc->msgIn + ADSTCPDataStart);
			if (DeviceInfo->ADSerror != 0) {
				ads_debug(ADSDebug,
					  "Error in the received packet: %s",
					  ADSerrorText(DeviceInfo->ADSerror));
				return 0x01;
			}
			*pVersion = DeviceInfo->Version;
			memcpy(pDevName, DeviceInfo->name, 16);
		}
	} else {
		if ((ADSDebug & ADSDebugAnalyze) != 0)
			analyze(dc->msgIn);
		return 0x01;
	}
	return 0;
}

int
ADSwriteBytes(ADSConnection * dc, int indexGroup, int offset, int length,
	      void *data)
{
	AMSheader *h1;
	ADSwriteRequest *rq;
	ADSpacket *p1 = (ADSpacket *) dc->msgOut;
	h1 = &(p1->amsHeader);
	_ADSsetupHeader(dc, h1);
	h1->commandId = cmdADSwrite;
	h1->dataLength = sizeof(ADSwriteRequest) - maxDataLen + length;
	_ADSDumpAMSheader(h1);

	p1->adsHeader.length = h1->dataLength + 32;
	p1->adsHeader.reserved = 0;

	rq = (ADSwriteRequest *) & p1->data;
	rq->indexGroup = indexGroup;
	rq->indexOffset = offset;
	rq->length = length;
	if (data != NULL) {
		memcpy(rq->data, data, length);
	}
	ads_debug(ADSDebugPacket, "Index Group:   %x\n", rq->indexGroup);
	ads_debug(ADSDebugPacket, "Index Offset:  %d\n", rq->indexOffset);
	ads_debug(ADSDebugPacket, "Data length:  %d\n", rq->length);
	_ADSwrite(dc);
	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn);
	if (dc->AnswLen > 0) {
//      res=dc->AnswLen;
		if ((ADSDebug & ADSDebugPacket) != 0) {
			_ADSDump("packet", dc->msgIn, dc->AnswLen);
		}
		if ((ADSDebug & ADSDebugAnalyze) != 0) {
			analyze(dc->msgIn);
		}
	}
	return 0;
}

/**
 * \brief Builds the necessary headers and analises the ADSserver ADSstate response
 * \param dc ADSConection handler
 * \param ADSstate Address of a variable that will receive the ADS status (see data type ADSSTATE).
 * \param devState Address of a variable that will receive the device status. 
 * \return Error code
 */
int
ADSreadState(ADSConnection * dc, unsigned short *ADSstate,
	     unsigned short *devState)
{
	AMSheader *h1;
	ADSpacket *p1 = (ADSpacket *) dc->msgOut;
	ADSpacket *p2;
	ADSstateResponse *StateResponse;

    /******* Send ******/
	/* Build the comunication headers */
	h1 = &(p1->amsHeader);
	_ADSsetupHeader(dc, h1);
	h1->commandId = cmdADSreadState;
	h1->dataLength = 0;
	if (ADSDebug & ADSDebugReadState)
		_ADSDumpAMSheader(h1);

	p1->adsHeader.length = h1->dataLength + AMSHeaderSIZE;
	p1->adsHeader.reserved = 0;
	/* sends the the packet */
	_ADSwrite(dc);

	/******* Receive ******/
	/*Reads the answer */
	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn);
	if (ADSDebug & ADSDebugReadState)
		analyze(dc->msgIn);

	if (ADSDebug & ADSDebugReadState & ADSDebugPacket)
		_ADSDump("ADSreadState Packet", dc->msgIn, dc->AnswLen);

	/* Analises the received packet */
	if (dc->AnswLen <= 0) {
		ads_debug(ADSDebugPrintErrors, "Read State Error: %s", 0xE);
		return 0xE;
	}
	p2 = (ADSpacket *) dc->msgIn;
	if (p2->amsHeader.errorCode != 0) {
		ads_debug(ADSDebugPrintErrors,
			  "ADSreadState Error: %s",
			  ADSerrorText(p2->amsHeader.errorCode));
		return p2->amsHeader.errorCode;
	}
	if (p2->amsHeader.commandId == cmdADSreadState) {
		StateResponse =
		    (ADSstateResponse *) (dc->msgIn + ADSTCPDataStart);
		*ADSstate = StateResponse->ADSstate;;
		*devState = StateResponse->devState;
	}

	return 0;
}

int
ADSwriteControl(ADSConnection * dc, int ADSstate, int devState, void *data,
		int length)
{
	AMSheader *h1;
	ADSwriteControlRequest *rq;
	ADSpacket *p1 = (ADSpacket *) dc->msgOut;
	h1 = &(p1->amsHeader);
	_ADSsetupHeader(dc, h1);
	h1->commandId = cmdADSwriteControl;
	h1->dataLength = sizeof(ADSwriteControlRequest);

	ads_debug(ADSDebug, "SIZE:  %d\n", h1->dataLength);
	_ADSDumpAMSheader(h1);

	p1->adsHeader.length = h1->dataLength + 32;
	p1->adsHeader.reserved = 0;

	rq = (ADSwriteControlRequest *) & p1->data;
	rq->ADSstate = ADSstate;
	rq->devState = devState;
	rq->length = 0;
	if (data != NULL) {
		memcpy(rq->data, data, length);
		rq->length = length;
		h1->dataLength += length;
	}

	_ADSwrite(dc);
	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn);
	if (dc->AnswLen > 0) {
		if ((ADSDebug & ADSDebugPacket) != 0) {
			_ADSDump("packet", dc->msgIn, dc->AnswLen);
		}
		if ((ADSDebug & ADSDebugAnalyze) != 0) {
			analyze(dc->msgIn);
		}
	}
	return 0;
}

int
ADSaddDeviceNotification(ADSConnection * dc,
			 int indexGroup, int offset, int length,
			 int transmissionMode, int maxDelay, int cycleTime)
{
	AMSheader *h1;
	ADSaddDeviceNotificationRequest *rq;
	ADSpacket *p1 = (ADSpacket *) dc->msgOut;
	h1 = &(p1->amsHeader);
	_ADSsetupHeader(dc, h1);
	h1->commandId = cmdADSaddDeviceNotification;
	h1->dataLength = sizeof(ADSaddDeviceNotificationRequest);

	ads_debug(ADSDebug, "SIZE:  %d\n", h1->dataLength);
	_ADSDumpAMSheader(h1);

	p1->adsHeader.length = h1->dataLength + 32;
	p1->adsHeader.reserved = 0;

	rq = (ADSaddDeviceNotificationRequest *) & p1->data;
	rq->indexGroup = indexGroup;
	rq->indexOffset = offset;
	rq->length = length;
	rq->transmissionMode = transmissionMode;
	rq->maxDelay = maxDelay;
	rq->cycleTime = cycleTime;
	_ADSwrite(dc);
	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn);
	if (dc->AnswLen > 0) {
		if ((ADSDebug & ADSDebugPacket) != 0) {
			_ADSDump("packet", dc->msgIn, dc->AnswLen);
		}
		if ((ADSDebug & ADSDebugAnalyze) != 0) {
			analyze(dc->msgIn);
		}
	}
	return 0;
}

int ADSparseNetID(const char *netIDstring, AMSNetID * id)
{
	const char *p;
	char *q;
	int i;
	q = (char *)id;
	p = netIDstring;
	for (i = 0; i < 6; i++) {
		q[i] = atoi(p);
		ads_debug(ADSDebug, "%d:  %d\n", i, q[i]);
		p = strchr(p, '.');
		if (p == NULL)
			return 5 - i;
		p++;
	}
	return 0;
}

/**
 * \brief Gets the local network address.
 * Fills the local AMSNetif.
 * \param id Structure with local AMSNetId.
 * \return error code.
 */
int ADSGetLocalAMSId(AMSNetID * id)
{
	struct ifaddrs *list;
	unsigned char b[4];
	unsigned long int netAddr;
	struct sockaddr_in *addrStruct;

	if (getifaddrs(&list) < 0) {
		return 0x01;
	}

	struct ifaddrs *cur;
	for (cur = list; cur != NULL; cur = cur->ifa_next) {
		if ((cur->ifa_addr->sa_family == AF_INET)
		    && (strcmp(cur->ifa_name, "lo") != 0)) {
			addrStruct = (struct sockaddr_in *)cur->ifa_addr;
			netAddr = ntohl(addrStruct->sin_addr.s_addr);
			memcpy((char *)&b, (char *)&netAddr, 4);
			*id = (AMSNetID) {
				{
			b[3], b[2], b[1], b[0], 1, 1}};
			if (ADSDebug)
				_ADSDumpAMSNetId(id);
			break;
		}
		if (cur->ifa_next == NULL)
			*id = (AMSNetID) {
			{
			127, 0, 0, 1, 1, 1}
			};
	}
	freeifaddrs(list);
	return 0;

}

/**
 * \brief Opens a new onnection to the Ads client.
 * This is an auxiliar function.
 * \param pAddr Structure with NetId and port number of the ADS server.
 * \param pMeAddr Structure with NetId and port number of the ADS client (local).
 * If the supplied pointer is NULL, then the address is automatically found and used.
 * \return An ADSConnection pointer.
 */
ADSConnection *AdsSocketConnect(int *socket_fd, PAmsAddr pAddr,
				PAmsAddr pMeAddr)
{

	struct sockaddr_in addr;
	socklen_t addrlen;
	char peer[12];
	int opt;
	ADSInterface *di;
	ADSConnection *dc;
	_ADSOSserialType fds;
	if (*socket_fd == 0)
		*socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	/* Build socket address */
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0xBF02);	/* ADS port 48898 */
	/* lazy convertion from byte array to socket adress format */
	sprintf(peer, "%d.%d.%d.%d", pAddr->netId.b[0], pAddr->netId.b[1],
		pAddr->netId.b[2], pAddr->netId.b[3]);
	inet_aton(peer, &addr.sin_addr);

	addrlen = sizeof(addr);

	/* connect to plc */
	if (connect(*socket_fd, (struct sockaddr *)&addr, addrlen) < 0) {
		ads_debug(ADSDebugOpen, "Socket error: %s \n", strerror(errno));
		return NULL;
	}
	ads_debug(ADSDebugOpen, "connected to %s", peer);
	errno = 0;
	opt = 1;
	if (setsockopt(*socket_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, 4) < 0) {
		ads_debug(ADSDebugOpen, "setsockopt %s\n", strerror(errno));
		return NULL;
	}

	fds.rfd = *socket_fd;
	fds.wfd = *socket_fd;
	di = ADSNewInterface(fds, pMeAddr->netId, pAddr->port, "test");
	dc = ADSNewConnection(di, pAddr->netId, pAddr->port);

	return dc;
}

/**
 * \brief Closes the connection socket opened by AdsSocketConnect
 * \return Error code
 */
int AdsSocketDisconnect(int *fd)
{
	if (*fd == 0) {
		return 0xD;	/* Port not connected */
	}
	close(*fd);
	*fd = 0;
	return 0;
}
