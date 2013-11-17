/*
 Implementation of BECKHOFF's ADS protocol.
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System
 May 2011.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of
 BECKHOFF Company. www.beckhoff.de

 Copyright (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2003.
 Copyright (C) Luis Matos (gass@otiliamatos.ath.cx) 2009.
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

#include <unistd.h>
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
#include "ads_io.h"
#include "debugprint.h"

AmsAddr 		meAddr = {{"\0"}, 0};		// filled in by AdsGetMeAddress()

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
		printf("%s\n", tmp_str);
		va_end(args);
	}
}

/**
 * A helper function to find out, what an errorcode returned by an
 * Ads*** function means...
 * The list of errorcodes is NOT complete!
 * @param err ADS errorcode
 * @return Returns a short text.
 */
char *ADSerrorText(int err)
{
	switch (err) {
		case 0:
			return "ok";
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
		case 0x050a:
			return "Router: not (yet) active";

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
		case 0x0754:
			return "invalid response received";
		case 0x0755:
			return "Sync port locked";


		case 0x274c:
			return "A socket operation was attempted to an unreachable host";
		case 0x274d:
			return "A connection attempt failed";

		default:
			return "Unknown error code";
	}
}

/**
 * Allocates space for ADSInterface and fills members with given params
 */
ADSInterface *_ADSNewInterface(int sd,
							  AmsNetId me,
							  int port,
							  char *nname)
{
	ADSInterface *di = (ADSInterface *) calloc(1, sizeof(ADSInterface));
	if (di) {
		di->sd = sd;
		di->name = nname;
		di->me = me;
		di->AMSport = port;
	}
	return di;
};

/**
 * Frees the allocated space for ADSInterface
 */
int _ADSFreeInterface(ADSInterface * di)
{
	free(di);
	return 0;
}

/**
 * Allocates space for a a new connection structure and fills members
 *	using an initialized ADSInterface.
 */
ADSConnection *_ADSNewConnection(ADSInterface * di, AmsNetId partner, int port)
{
	ADSConnection *dc = (ADSConnection *) calloc(1, sizeof(ADSConnection));
	if (dc) {
		dc->iface = di;
		dc->partner = partner;
		dc->AMSport = port;
	}
	return dc;
}

/**
 * Frees the allocated space for ADSConnection
 */
void ADSFreeConnection(ADSConnection *dc)
{
	_ADSFreeInterface(dc->iface);
	free(dc);
}

/**
    Setup an AMSHeader using an initialized ADSConnection.
 */
void _ADSsetupAmsHeader(ADSConnection *dc, AMSheader *h)
{
	h->targetId 	= dc->partner;
	h->targetPort 	= dc->AMSport;
	h->sourceId 	= dc->iface->me;
	h->sourcePort 	= dc->iface->AMSport;
	h->stateFlags 	= 4;
	h->errorCode 	= 0;
	h->invokeId 	= ++dc->invokeId;
};

/**
 * \brief Returns the local NetId and port number.
 * AdsGetMeAddress stores it in the global variable meAddr.
 * If already have determined it, simply copy meAddr to pAddr.
 * \param port  port number of an Ads port that had previously been opened with
 *				AdsPortOpenEx or AdsPortOpen..
 * \param pAddr Pointer to the structure of type AmsAddr.
 * \return Returns the function's error status.
 */
long AdsGetMeAddress(PAmsAddr pAddr, int AmsPort)
{
	struct ifaddrs *list, *cur;
	unsigned long int netAddr;
	struct sockaddr_in *addrStruct;

	if(meAddr.netId.b[0] == 0){
		if (getifaddrs(&list) < 0) {
			return 0x1; // internal error
		}

		for (cur = list; cur != NULL; cur = cur->ifa_next) {
			if ((cur->ifa_addr->sa_family == AF_INET) && (strcmp(cur->ifa_name, "lo") != 0)) {
				addrStruct = (struct sockaddr_in *) cur->ifa_addr;
				netAddr = addrStruct->sin_addr.s_addr;
				memcpy((char *) &(meAddr.netId), (char *) &netAddr, 4);
				meAddr.netId.b[4] = 1;
				meAddr.netId.b[5] = 1;
				break;
			}
			if (cur->ifa_next == NULL){
				return 0xa; // no network interface ????
			}
		}
		freeifaddrs(list);
		meAddr.port = AmsPort;
	}

	memcpy((char *)pAddr, (char *)&meAddr, sizeof(AmsAddr));
	return 0x0;
}

/**
 * This is an interface to AdsAPI.c.
 * Used by AdsSyncReadReqEx()
 */
int ADSreadBytes(ADSConnection *dc,
				 unsigned long indexGroup, unsigned long offset,
				 unsigned long length, void *buffer,
				 unsigned long *pnRead)
{
	AMSheader 		*h1;
//	AMS_TCPheader 	*h2; not used
	ADSpacket 		*p1;
	ADSpacket 		*p2;
	ADSreadRequest  *rq;
	ADSreadResponse *rr;
	int				rc;
	int				nErr;

	MsgOut(MSG_TRACE, "ADSreadBytes() called\n");

	p1 = (ADSpacket *) dc->msgOut;
	h1 = &(p1->amsHeader);
//	h2 = &(p1->adsHeader); not used
	_ADSsetupAmsHeader(dc, h1);

	h1->commandId = cmdADSread;
	h1->dataLength = sizeof(ADSreadRequest);

	p1->adsHeader.length = sizeof(AMSheader) + h1->dataLength;
	p1->adsHeader.reserved = 0;

	rq = (ADSreadRequest *) &p1->data;
	rq->indexGroup = indexGroup;
	rq->indexOffset = offset;
	rq->length = length;

	MsgOut(MSG_PACKET_V, MsgStr("Index Group:   0x%x\n", rq->indexGroup));
	MsgOut(MSG_PACKET_V, MsgStr("Index Offset:  %d\n", rq->indexOffset));
	MsgOut(MSG_PACKET_V, MsgStr("Data length:   %d\n", rq->length));

	MsgAnalyzePacket("ADSreadBytes", p1);

	rc = _ADSWritePacket(dc->iface, p1, &nErr);
	if(rc <= 0){
        MsgOut(MSG_ERROR, "ADSreadBytes() failed().\n");
		return( _ADStranslateWrError(rc, nErr));
	}

	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn, &nErr);
	if (dc->AnswLen > 0 && nErr == 0 ) {
		p2 = (ADSpacket *) dc->msgIn;
// 		if (p2->amsHeader.commandId == cmdADSread && p2->amsHeader.errorCode == 0) {
		if (p2->amsHeader.commandId == cmdADSread) {
			MsgDumpPacket("ADSreadBytes", dc->msgIn, dc->AnswLen);
			MsgAnalyzePacket("ADSreadBytes", (ADSpacket *)dc->msgIn);

			rr = (ADSreadResponse *) (dc->msgIn + 38);
/*			if (rr->result != 0){
#ifdef LOG_ALL_MESSAGES
				syslog(LOG_USER | LOG_ERR,
					   "ADSreadBytes(): ADSreadResponse() failed with error 0x%x.",
					   rr->result);
#endif
				MsgOut(MSG_ERROR,
					   MsgStr("ADSreadBytes(): ADSreadResponse() failed with error 0x%x.\n",
							  rr->result));
				return rr->result;
			}*/
			dc->dataPointer = dc->msgIn + 46;
			dc->AnswLen = rr->length;
			if (buffer != NULL) {
				if(rr->length > length){
					char strBuf[256];
					sprintf(strBuf, "ADSreadBytes() failed(): Buffer sized %lu bytes, got %u bytes.", length, rr->length);
					MsgOut(MSG_ERROR, MsgStr("%s\n", strBuf));
					return(0x705); // parameter size not correct
				}
				memcpy(buffer, dc->dataPointer, rr->length);
			}
			if(pnRead != NULL)
				*pnRead = rr->length;

			MsgOut(MSG_DEVEL,
				   MsgStr("ADSreadBytes() invokeId=%d\n", p2->amsHeader.invokeId));
			MsgOut(MSG_TRACE, "ADSreadBytes() returns 0 (OK)\n");
// 			return 0;

// printf("rr->result=0x%x, p2->amsHeader.errorCode=0x%x\n", rr->result, p2->amsHeader.errorCode);

			return rr->result;
		}
		else
			return p2->amsHeader.errorCode;
	}

	MsgOut(MSG_ERROR, "ADSreadBytes() failed().\n");
	return(_ADStranslateRdError(dc->AnswLen, nErr));
}

/**
 * This is an interface to AdsAPI.c.
 * Used by AdsSyncWriteReq().
 */
int ADSwriteBytes(ADSConnection *dc,
				  int indexGroup, int offset,
				  int length, void *data)
{
	ADSpacket			*p1;
	ADSpacket			*p2;
	AMSheader			*h1;
//	AMS_TCPheader		*h2; not used
	ADSwriteRequest		*rq;
	ADSwriteResponse	*wr;
	int					rc;
	int					nErr;

	MsgOut(MSG_TRACE, "ADSwriteBytes() called\n");

	p1 = (ADSpacket *) dc->msgOut;
	h1 = &(p1->amsHeader);
//	h2 = &(p1->adsHeader); not used

	_ADSsetupAmsHeader(dc, h1);
	h1->commandId = cmdADSwrite;
	h1->dataLength = sizeof(ADSwriteRequest) - MAXDATALEN + length;
	p1->adsHeader.length = sizeof(AMSheader) + h1->dataLength;
	p1->adsHeader.reserved = 0;

	rq = (ADSwriteRequest *) & p1->data;
	rq->indexGroup = indexGroup;
	rq->indexOffset = offset;
	rq->length = length;
	if (data != NULL)
		memcpy(rq->data, data, length);

	MsgOut(MSG_PACKET_V, MsgStr("Index Group:   %x\n", rq->indexGroup));
	MsgOut(MSG_PACKET_V, MsgStr("Index Offset:  %d\n", rq->indexOffset));
	MsgOut(MSG_PACKET_V, MsgStr("Data length:   %d\n", rq->length));

	MsgAnalyzePacket("ADSwriteBytes()", p1);
	rc = _ADSWritePacket(dc->iface, p1, &nErr);
	if(rc <= 0){
		MsgOut(MSG_ERROR, "ADSwriteBytes() failed().\n");
		return( _ADStranslateWrError(rc, nErr));
	}

	/*Reads the answer */
	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn, &nErr);
	if (dc->AnswLen >= (sizeof(AMS_TCPheader) + sizeof(AMSheader))) {
		p2 = (ADSpacket *) dc->msgIn;
		if(nErr == 0 ) {
			MsgAnalyzePacket("ADSwriteBytes()", (ADSpacket*)dc->msgIn);
			wr = (ADSwriteResponse *) (dc->msgIn + 38);
			MsgOut(MSG_TRACE,
				MsgStr("ADSwriteBytes() returns 0x%x (0 means OK)\n", wr->result));
			return wr->result;
		}
		else
			return p2->amsHeader.errorCode;
	}

	/* if there is an error */
	MsgOut(MSG_ERROR, "ADSwriteBytes() failed().\n");
	return(_ADStranslateRdError(dc->AnswLen, nErr));
}

/**
 * This is an interface to AdsAPI.c.
 * Used by ADSreadDeviceInfo().
 */
int ADSreadDeviceInfo(ADSConnection * dc, char *pDevName, PAdsVersion pVersion)
{
	AMSheader 		*h1;
//	AMS_TCPheader 	*h2; not used
	ADSpacket 		*p1;
	ADSpacket 		*p2;
	ADSdeviceInfo 	*DeviceInfo;
	int				rc;
	int				nErr;

	MsgOut(MSG_TRACE, "ADSreadDeviceInfo() called\n");

	p1 = (ADSpacket *) dc->msgOut;
	h1 = &(p1->amsHeader);
//	h2 = &(p1->adsHeader); not used
	_ADSsetupAmsHeader(dc, h1);
	h1->commandId = cmdADSreadDevInfo;
	h1->dataLength = 0;
	p1->adsHeader.length = sizeof(AMSheader) + h1->dataLength;
	p1->adsHeader.reserved = 0;

	MsgAnalyzePacket("ADSreadDeviceInfo()", p1);
	rc = _ADSWritePacket(dc->iface, p1, &nErr);
	if(rc <= 0){
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR, "ADSreadDeviceInfo() failed().");
#endif
		MsgOut(MSG_ERROR, "ADSreadDeviceInfo() failed().\n");
		return( _ADStranslateWrError(rc, nErr));
	}

	/*Reads the answer */
	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn, &nErr);
	if (dc->AnswLen > 0 && nErr == 0 ) {
		p2 = (ADSpacket *) dc->msgIn;
		if (p2->amsHeader.commandId == cmdADSreadDevInfo) {
			MsgAnalyzePacket("ADSreadDeviceInfo()", (ADSpacket*)dc->msgIn);
			DeviceInfo = (ADSdeviceInfo *) (dc->msgIn + 38);
			*pVersion = DeviceInfo->Version;
			memcpy(pDevName, DeviceInfo->name, 16);
			MsgOut(MSG_TRACE,
				   MsgStr("ADSreadDeviceInfo() returns 0x%x (0 means OK)\n", DeviceInfo->result));
			return DeviceInfo->result;
		}
		else
			return p2->amsHeader.errorCode;
	}

	/* if there is an error */
#ifdef LOG_ALL_MESSAGES
	syslog(LOG_USER | LOG_ERR, "ADSreadDeviceInfo() failed().");
#endif
	MsgOut(MSG_ERROR, "ADSreadDeviceInfo() failed().\n");
	return(_ADStranslateRdError(dc->AnswLen, nErr));

}

/**
 * This is an interface to AdsAPI.c.
 * Used by AdsSyncReadWriteReqEx().
 */
int ADSreadWriteBytes(ADSConnection * dc,
					  unsigned long indexGroup, unsigned long offset,
					  unsigned long readLength, void *readBuffer,
					  unsigned long writeLength, void *writeBuffer,
					  unsigned long *pnRead)
{
	AMSheader 				*h1;
//	AMS_TCPheader 			*h2; not used
	ADSreadWriteRequest 	*rq;
	ADSreadWriteResponse	*rr;
	int						rc;
	int						nErr;

	ADSpacket *p1 = (ADSpacket *) dc->msgOut;
	ADSpacket *p2;

	MsgOut(MSG_TRACE, "ADSreadWriteBytes() called\n");

	h1 = &(p1->amsHeader);
//	h2 = &(p1->adsHeader); not used

	_ADSsetupAmsHeader(dc, h1);
	h1->commandId = cmdADSreadWrite;
	h1->dataLength = sizeof(ADSreadWriteRequest) - MAXDATALEN + writeLength;

//	h2->length = h1->dataLength + sizeof(AMSheader); not used
//	h2->reserved = 0; not used

	rq = (ADSreadWriteRequest *) & p1->data;
	rq->indexGroup = indexGroup;
	rq->indexOffset = offset;
	rq->writeLength = writeLength;
	rq->readLength = readLength;
	if (writeBuffer != NULL) {
		memcpy(rq->data, writeBuffer, writeLength);
	}
	MsgOut(MSG_PACKET_V, MsgStr("Index Group:   0x%x\n", rq->indexGroup));
	MsgOut(MSG_PACKET_V, MsgStr("Index Offset:  %d\n", rq->indexOffset));
	MsgOut(MSG_PACKET_V, MsgStr("Data length:   %d\n", rq->writeLength));

	MsgAnalyzePacket("ADSreadWriteBytes()", p1);
	rc = _ADSWritePacket(dc->iface, p1, &nErr);
	if(rc <= 0){
		MsgOut(MSG_ERROR, "ADSreadWriteBytes() failed().\n");
		return( _ADStranslateWrError(rc, nErr));
	}

	/*Reads the answer */
	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn, &nErr);
	if (dc->AnswLen > 0 && nErr == 0 ) {
		p2 = (ADSpacket *) dc->msgIn;
		if (p2->amsHeader.commandId == cmdADSreadWrite) {

			MsgAnalyzePacket("ADSreadWriteBytes()", p2);
			rr = (ADSreadWriteResponse *) (dc->msgIn + 38);
			if(rr->length > readLength){
				char strBuf[256];
				sprintf(strBuf, "ADSreadWriteBytes() failed(): Max Read: %lu, got %u", readLength, rr->length);
				return(0x705); // parameter size not correct
			}
// 			memcpy(readBuffer, rr->data, readLength);
			memcpy(readBuffer, rr->data, rr->length);
			if(pnRead != NULL)
				*pnRead = rr->length;

			MsgOut(MSG_TRACE,
				   MsgStr("ADSreadBytes() returns 0x%x (0 means OK)\n", rr->result));
			return rr->result;
		}
		else
			return p2->amsHeader.errorCode;
	}

	/* if there is an error */
	MsgOut(MSG_ERROR, "ADSreadWriteBytes() failed().\n");
	return(_ADStranslateRdError(dc->AnswLen, nErr));
}

/**
 * \brief Builds the necessary headers and analises the ADSserver ADSstate response
 * This is an interface to AdsAPI.c.
 * Used by AdsSyncReadStateReq().
 * \param dc ADSConection handler
 * \param ADSstate Address of a variable that will receive the ADS status (see data type ADSSTATE).
 * \param devState Address of a variable that will receive the device status. 
 * \return Error code
 */
int ADSreadState(ADSConnection * dc,
				 unsigned short *ADSstate,
				 unsigned short *devState)
{
	AMSheader 			*h1;
//	AMS_TCPheader 		*h2; not used
	ADSpacket 			*p1;
	ADSpacket 			*p2;
	ADSstateResponse 	*StateResponse;
	int					rc;
	int					nErr;

	MsgOut(MSG_TRACE, "ADSreadState() called\n");

	p1 = (ADSpacket *) dc->msgOut;
	h1 = &(p1->amsHeader);
//	h2 = &(p1->adsHeader); not used
	_ADSsetupAmsHeader(dc, h1);
	h1->commandId = cmdADSreadState;
	h1->dataLength = 0;
	p1->adsHeader.length = sizeof(AMSheader) + h1->dataLength;
	p1->adsHeader.reserved = 0;
	MsgAnalyzePacket("ADSreadState()", p1);

	/* sends the the packet */
	rc = _ADSWritePacket(dc->iface, p1, &nErr);
	if(rc <= 0){
		MsgOut(MSG_ERROR, "ADSreadState failed().\n");
		return( _ADStranslateWrError(rc, nErr));
	}

	/*Read the answer */
	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn, &nErr);
	MsgDumpPacket("ADSreadState()", dc->msgIn, dc->AnswLen);
	MsgAnalyzePacket("ADSreadState()", (ADSpacket*)dc->msgIn);
	if (dc->AnswLen > 0 && nErr == 0 ) {
		p2 = (ADSpacket *) dc->msgIn;
		if (p2->amsHeader.commandId == cmdADSreadState) {
			MsgAnalyzePacket("ADSreadState()", p2);
			StateResponse = (ADSstateResponse *) (dc->msgIn + 38);
			*ADSstate = StateResponse->ADSstate;;
			*devState = StateResponse->devState;
			MsgOut(MSG_TRACE,
				   MsgStr("ADSreadState() returns 0x%x (0 means OK)\n", StateResponse->result));
			return StateResponse->result;
		}
		else
			return p2->amsHeader.errorCode;
	}

	/* if there is an error */
	MsgOut(MSG_ERROR, "ADSreadState failed().\n");
	return(_ADStranslateRdError(dc->AnswLen, nErr));
}

/**
 * This is an interface to AdsAPI.c.
 * Used by AdsSyncReadStateReq().
 */
int ADSwriteControl(ADSConnection *dc,
					int ADSstate,
					int devState,
					void *data, int length)
{
	AMSheader 				*h1;
//	AMS_TCPheader 			*h2; not used
	ADSpacket 				*p1;
	ADSpacket				*p2;
	ADSwriteControlRequest 	*rq;
	ADSwriteResponse		*wr;
	int						rc;
	int						nErr;

	MsgOut(MSG_TRACE, "ADSwriteControl() called\n");

	p1 = (ADSpacket *) dc->msgOut;
	h1 = &(p1->amsHeader);
//	h2 = &(p1->adsHeader); not used

	_ADSsetupAmsHeader(dc, h1);
	h1->commandId = cmdADSwriteControl;
	h1->dataLength = sizeof(ADSwriteControlRequest);
	p1->adsHeader.length = sizeof(AMSheader) + h1->dataLength;
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

	MsgAnalyzePacket("ADSwriteControl()", p1);
	rc = _ADSWritePacket(dc->iface, p1, &nErr);
	if(rc <= 0){
		MsgOut(MSG_ERROR, "ADSwriteControl() failed().\n");
		return( _ADStranslateWrError(rc, nErr));
	}


	dc->AnswLen = _ADSReadPacket(dc->iface, dc->msgIn, &nErr);
	p2 = (ADSpacket *) dc->msgIn;
	if (dc->AnswLen > 0 && nErr == 0 ) {
		MsgAnalyzePacket("ADSwriteControl()", p2);
		wr = (ADSwriteResponse *) (dc->msgIn + 38);
		MsgOut(MSG_TRACE,
			   MsgStr("ADSwriteControl() returns 0x%x (0 means OK)\n", wr->result));
		return wr->result;
	}
	else{
		return p2->amsHeader.errorCode;
	}
	/* if there is an error */
	MsgOut(MSG_ERROR, "ADSwriteControl() failed().\n");
	return(_ADStranslateRdError(dc->AnswLen, nErr));
}

/**
 * This is an internal function
 * Input: netIDstring, something like "127.0.0.1.1.1"
 * Output: id, the input converted to an AmsNetID struct.
 */
int _ADSparseNetID(const char *netIDstring, AmsNetId *id)
{
	const char *p;
	char *q;
	int i;

	MsgOut(MSG_TRACE, "_ADSparseNetID() called\n");
	q = (char *) id;
	p = netIDstring;
	for (i = 0; i < 6; i++) {
		q[i] = atoi(p);
		MsgOut(MSG_TRACE_V, MsgStr("%d:  %d\n", i, q[i]));
		p = strchr(p, '.');
		if (p == NULL){
			MsgOut(MSG_TRACE_V, MsgStr("_ADSparseNetID() returns %d\n", 5 - i));
			return 5 - i;
		}
		p++;
	}
	MsgOut(MSG_TRACE, MsgStr("_ADSparseNetID() returns %d\n", 0));
	return 0;
}

/**
 * \brief Gets the local network address.
 * Fills the local AMSNetif.
 * \param id Structure with local AmsNetId.
 * \return error code.
 */
int ADSGetLocalAMSId(AmsNetId * id)
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
		if (cur->ifa_addr != NULL) {
			if ((cur->ifa_addr->sa_family == AF_INET)
		    	&& (strcmp(cur->ifa_name, "lo") != 0)) {
				addrStruct = (struct sockaddr_in *)cur->ifa_addr;
				netAddr = ntohl(addrStruct->sin_addr.s_addr);
				memcpy((char *)&b, (char *)&netAddr, 4);
				*id = (AmsNetId) {
				{
				b[3], b[2], b[1], b[0], 1, 1}};
				break;
			}
		}
		if (cur->ifa_next == NULL)
			*id = (AmsNetId) {
			{
			127, 0, 0, 1, 1, 1}
			};
	}
	freeifaddrs(list);
	return 0;

}

int _ADStranslateWrError(int rc, int nErr)
{
/*	syslog(LOG_USER | LOG_ERR,
		   "_ADSWritePacket() returns %d, errno %d.", rc, nErr);
	MsgOut(MSG_ERROR,
		   MsgStr("_ADSWritePacket() returns %d, errno %d.\n", rc, nErr));*/
	switch(rc){
		case 0:		// system error
			return 0x1a; //TODO translate nErr (=errno) int ADS return code
		case -3:	// internal error
		case -4:	// error flag set in ADSinterface
			return 0x1;
	}
	//NOT REACHED
	return 0;
}

int _ADStranslateRdError(int rc, int nErr)
{
/*	syslog(LOG_USER | LOG_ERR,
		   "_ADSReadPacket() returns %d, errno %d.", rc, nErr);
	MsgOut(MSG_ERROR,
		   MsgStr("_ADSReadPacket() returns %d, errno %d.\n", rc, nErr));*/
	switch(rc){
		case 0:		// system error
			return 0x1a; //TODO translate nErr (=errno) int ADS return code
		case -1:	// time out
			return 0x15;
		case -2:	// worker shut down
			return 0x50a;
		case -3:	// internal error
		case -4:	// error flag set in ADSinterface
			return 0x1;
	}
	// NOT REACHED
	return(0);
}
