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

#ifndef __DEBUGPRINT_H__
#define __DEBUGPRINT_H__


#define MSG_ERROR 					01		// Error conditions
#define MSG_INFO 					02		// Run time infos
#define MSG_TRACE 					04		// Programm flow
#define MSG_TRACE_V					010		// Programm flow, verbose
#define MSG_PACKET 					020		// AMS Packet
#define MSG_PACKET_V				040		// AMS Packet, verbose
#define MSG_SOCKET					0100	// socket
#define MSG_SOCKET_V				0200	// socket, verbose
#define MSG_NOTIFICATION			0400	// notification messages
#define MSG_ROUTING					01000	// routing messages

#define MSG_ALL_SIMPLE				01527	// everything, but not verbose
#define MSG_ALL 					01777

#define MSG_DEVEL					02000	// development...


#ifndef DEBUGPRINT			// undefine DEBUGPRINT for a release build
#define MsgOut(t,m)  					((void)0) // prints string "m"
#define MsgCnt(t,m)  					((void)0)
#define MsgStr		 					((void)0) // like "printf()"
#define	MsgDumpPacket(name,v,len) 		((void)0) // hex dump, no interpreting done
#define MsgAnalyzePacket(name, p)		((void)0) // interprete AMS packet (header + data)
#define MsgAnalyzeHeader(t,h) 			((void)0) // interprete AMS header

#define	MsgADSDumpDec(t,name,v,len) 	((void)0)
#define MsgDumpAMSaddr(t,a) 			((void)0)

#else						// define DEBUGPRINT for a debug build
// in case we include it in an application
#include <stdbool.h>

#include "AdsDEF.h"
#include "ads.h"
#include "ads_io.h"
#include "ads_connect.h"

#define logout 						stderr
#define maxDataDump 				1524 // ethernet packet length maximum

#define MsgOut(t,m)  				_msgout(__FILE__, __LINE__, t, m)
#define MsgCnt(t,m)  				_msgcnt(t, m)
#define MsgStr       				_msgoutp
#define MsgDumpPacket(name,v,len)  	_msgDumpPacket(__FILE__, __LINE__, name, v, len)
#define MsgAnalyzePacket(name, p)	_msgAnalyzePacket(__FILE__, __LINE__, name, p)
#define MsgAnalyzeHeader(t,h) 		_msgAnalyzeHeader(__FILE__, __LINE__, t, h)
#define MsgDumpAMSaddr(t,a) 		_msgDumpAMSaddr(__FILE__, __LINE__, t, a)

void  _ADSDumpAMSNetId(char *szFile, int iLineNo, int iType, AmsNetId *id);
char* _ADSCommandName(int c);
char* _ADSStateName(int c);
void  _msgDumpAMSaddr(char *szFile, int iLineNo, int iType, PAmsAddr paddr);
char *_msgAmsNetID2string(PAmsNetId pNetid);
void  _msgout(char *szFile, int iLineNo, int iType, char *szMsg);
void _msgcnt(int iType, char *szMsg);
char* _msgoutp(char *fmt, ...);
void  _msgDumpPacket(char *szFile, int iLineNo, char *funcName, void *v, int len);
void  _msgAnalyzeHeader(char *szFile, int iLineNo, int iType,  AMSheader *h);
void  _msgAnalyzePacket(char *szFile, int iLineNo, char* funcName, void *pv);
void  _doDump(void *v, int len, bool doHex);
char* _fileFromPath(char * path);

#endif /* DEBUGPRINT */

#endif /* __DEBUGPRINT_H__ */
