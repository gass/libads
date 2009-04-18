/*
 Definitions for BECKHOFF's ADS protocol. 
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
#ifdef __cplusplus

extern "C" {
#endif

#ifndef __ADS
#define __ADS

#include "AdsDEF.h"

/* 
    Some simple types:
*/

#ifdef LINUX
#define DECL2
#define EXPORTSPEC
typedef struct dost {
    int rfd;
    int wfd;
//    int connectionType;
} _ADSOSserialType;
#include <stdlib.h>
#define tmotype int
#define OS_KNOWN	// get rid of nested ifdefs.
#endif    

#ifdef BCCWIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#define DECL2 __stdcall
#define tmotype int

#ifdef DOEXPORT
#define EXPORTSPEC __declspec (dllexport) 
#else
#define EXPORTSPEC __declspec (dllimport) 
#endif

typedef struct dost {
    HANDLE rfd;
    HANDLE wfd;
//    int connectionType;
} _ADSOSserialType;

#define OS_KNOWN
#endif

#ifdef DOS
#include <stdio.h>
#include <stdlib.h>
//#define DECL2 WINAPI
// #define DECL2
#define DECL2 __cedcl
#define EXPORTSPEC

typedef struct dost {
    int rfd;
    int wfd;
} _ADSOSserialType;
#define OS_KNOWN
#endif

#ifdef AVR
#include <stdio.h>
#include <stdlib.h>
#define DECL2
#define EXPORTSPEC
typedef struct dost {
    int rfd;
    int wfd;
} _ADSOSserialType;
#define tmotype long
#define OS_KNOWN
#endif

#ifndef OS_KNOWN
#error Fill in what you need for your OS or API.
#endif

#define uc unsigned char //characters
#define us unsigned short 
#define u32 unsigned int

#define USHORT unsigned short 
#define DWORD unsigned int


#pragma pack (1)

typedef struct _AMS_TCPheader {
    us	reserved;	// must be zero for now;
    u32 length;		//length in bytes
} AMS_TCPheader;



/**
    The AMS header.
*/
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
				// same value
} AMSheader;

/**
 * \brief Ads Command Id for AmsHeader
 * Other commands are not defined or are used internally. Therefore the Command Id is only allowed to contain the above enumerated values!
 */
enum nAdsCommandId {
	cmdADSinvalid			=0x0000,
	cmdADSreadDevInfo		=0x0001,
	cmdADSread			=0x0002,
	cmdADSwrite			=0x0003,
	cmdADSreadState			=0x0004,
	cmdADSwriteControl		=0x0005,
	cmdADSaddDeviceNotification	=0x0006,
	cmdADSdeleteDeviceNotification	=0x0007,
	cmdADSdevNotify			=0x0008,
	cmdADSreadWrite			=0x0009
}AdsCommandId;
/*
    Definitions of bits in stateFlags:
*/
#define sfAMSresponse	0x001		// zero means request
#define sfAMScommand	0x004		// indicates data exchange via ADS command
#define sfAMSudp	0x400		// indicates transport over UDP (0 mens TCP)

#define maxDataLen 1524			// just equal to ethernet packet length

typedef struct _ADSpacket {
    AMS_TCPheader	adsHeader;
    AMSheader		amsHeader;
    char data[maxDataLen];
} ADSpacket;


typedef struct _ADSdeviceInfo {
    u32 ADSerror;
    AdsVersion Version;
    char name[16];	// rely on 16 byte, not terminating #0
} ADSdeviceInfo;

typedef struct _ADSreadRequest {
    u32 indexGroup;	// index Group the data belongs to
    u32 indexOffset;	// offset within index Group 
    u32 length;		// length in bytes to be read
} ADSreadRequest;

/*
    Definitions for index Group:
*/
#define igADSfirstReserved	0x0000
#define igADSlastReserved	0x0FFF
#define igADSParameter		0x1000
#define igADSState		0x2000
#define igADSdevFunction	0x3000
#define igADScommonFunction	0x4000	//contains functions to access PLC process data image
#define igADSfirstReservedExt	0x6000	// first iG reserved for extensions
#define igADSlastReservedExt	0xEFFF	// last iG reserved for extensions
#define igADSfirstTwinCATsysService	0xF000	// general TwinCAT ADS system services...
#define igADSlastTwinCATsysService	0xFFFF	// ...containing functions to access PLC I/O image


#define errADSok	0x0000
#define errADSInternal	0x0001
#define errADSNoRuntime	0x0002
#define errADSAllocLockedMemory	0x0003


typedef struct _ADSreadResponse {
    u32 result;		// error code
    u32 length;		// length in bytes of response
    char data[maxDataLen];
} ADSreadResponse;

typedef struct _ADSwriteRequest {
    u32 indexGroup;
    u32 indexOffset;
    u32 length;		// length in bytes of response
    char data[maxDataLen];
} ADSwriteRequest;

typedef struct _ADSwriteResponse {
    u32 result;		// error code
} ADSwriteResponse;


typedef struct _ADSstateResponse {
    u32 result;
    us	ADSstate;
    us devState;
} ADSstateResponse;

typedef struct _ADSwriteControlRequest {
    us	ADSstate;
    us devState;
    u32 length;
    uc data[];
} ADSwriteControlRequest;


typedef struct _ADSaddNotificationRequest {
    u32 indexGroup;	// index Group the data belongs to
    u32 indexOffset;	// offset within index Group 
    u32 length;		// length in bytes to be read
    u32 transmissionMode; //
    u32 maxDelay;	// after this time the notification will be called up.
			// unit is 100ns
    u32 cycleTime;	// after this time the server teests whether values did change
			// unit is 100ns			
    uc reserved[16];
} ADSaddDeviceNotificationRequest;


typedef struct _ADSaddNotificationResponse {
    u32 result;
    u32 notificationHandle;	// offset within index Group 
} ADSaddDeviceNotificationResponse;


typedef struct _ADSreadWriteRequest {
    u32 indexGroup;
    u32 indexOffset;
    u32 readLength;		// length in bytes of response
    u32 writeLength;		// length in bytes of response
    char data[maxDataLen];
} ADSreadWriteRequest;

typedef struct _ADSreadWriteResponse {
    u32 result;		// length in bytes of response
    u32 length;		// length in bytes of response
    char data[maxDataLen];
} ADSreadWriteResponse;

/*
    Library specific stuff:
*/

extern int ADSDebug;

#define ADSDebugOpen 0x10
#define ADSDebugPacket 0x20
#define ADSDebugByte 0x40
#define ADSDebugAnalyze 0x80
#define ADSDebugPrintErrors 0x8
#define ADSDebugAll ADSDebugOpen|ADSDebugPacket|ADSDebugByte|ADSDebugAnalyze|ADSDebugPrintErrors


/* 
    This is a wrapper for the serial or network interface. 
*/
typedef struct {
    _ADSOSserialType fd; // some handle for the serial/network interface
    int error;	// Set when read/write errors occur. You will have to do something
		// specific to your OS to make transort work again.
    int timeout;// Timeout in microseconds used in transort.
    char * name;// this	name is used in error outut, so you can identify the interface	
    AMSNetID me;
    int AMSport;
}
ADSInterface;

/* 
    This holds data for a connection;
*/
typedef struct {
    ADSInterface * iface; // pointer to used interface
    int AnswLen;	// length of last message
    int invokeID;	// packetNumber in transport layer
    void * dataPointer;	// pointer to result data, if present
    uc msgIn[maxDataLen];
    uc msgOut[maxDataLen];
    AMSNetID partner;
    int AMSport;
    int invokeId;
}
ADSConnection;

/** 
    This will setup a new interface structure from an initialized
    serial interface's handle and a name.
*/
EXPORTSPEC ADSInterface * DECL2 ADSNewInterface(_ADSOSserialType nfd, AMSNetID me, int port, char * nname);

/** 
    This will setup a new connection structure using an initialized
    ADSInterface and PLC's MPI address.
*/
EXPORTSPEC ADSConnection * DECL2 ADSNewConnection(ADSInterface * di,AMSNetID partner, int port);

/**
    Hex dump:
*/
EXPORTSPEC void DECL2 _ADSDump(char * name, void * v,int len);


EXPORTSPEC void DECL2 _ADSDumpAMSNetId(AMSNetID * id);

EXPORTSPEC void DECL2 _ADSDumpAMSheader(AMSheader * h);

/**
    Naming
*/
EXPORTSPEC char * DECL2 ADSCommandName(int c);

EXPORTSPEC char * DECL2 ADSerrorText(int err);

EXPORTSPEC int DECL2 _ADSReadOne(ADSInterface * di, uc *b);

EXPORTSPEC int DECL2 _ADSReadPacket(ADSInterface * di,uc *b);

EXPORTSPEC void DECL2 analyze(uc * p1);

EXPORTSPEC int DECL2 ADSreadBytes(ADSConnection *dc, int indexGroup, int offset, int length, void * buffer);
EXPORTSPEC int DECL2 ADSreadDeviceInfo(ADSConnection *dc, char * pDevName, PAdsVersion pVersion );
EXPORTSPEC int DECL2 ADSwriteBytes(ADSConnection *dc, int indexGroup, int offset, int length, void * data);
EXPORTSPEC int DECL2 ADSreadState(ADSConnection *dc, unsigned short * ADSstate, unsigned short * devState);
EXPORTSPEC int DECL2 ADSwriteControl(ADSConnection *dc, int ADSstate, int devState, void * data, int length);
EXPORTSPEC int DECL2 ADSaddDeviceNotification(ADSConnection *dc, 
    int indexGroup, int offset, int length,
    int transmissionMode, int maxDelay, int cycleTime);
EXPORTSPEC int DECL2 ADSreadWriteBytes(ADSConnection *dc,
					int indexGroup,
					int offset,
					int readLength,
					void *readBuffer,
					int writeLength,
					void *writeBuffer );
EXPORTSPEC int DECL2 _ADSwrite(ADSConnection *dc);

EXPORTSPEC int DECL2 ADSparseNetID(const char * NetIDstring, AMSNetID * id);

#endif /* __ADS */

#ifdef __cplusplus
//#ifdef CPLUSPLUS
 }
#endif

/*
    Changes:
    03/12/2005 	added ADSparseNetID
*/
