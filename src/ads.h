/*
 Definitions for BECKHOFF's ADS protocol. 
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System Nov 2002.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of BECKHOFF 
 Company. www.beckhoff.de

 (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2003.
 (C) Luis Matos (gass@otiliamatos.ath.cx) 2012.

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
#include <stdio.h>

/* 
    Some simple types:
*/

#ifdef __linux__
    typedef struct dost {
	int rfd;
	int wfd;
//    int connectionType;
    } _ADSOSserialType;
#include <stdlib.h>
#define tmotype int
#endif


#define uc unsigned char	//characters
#define us unsigned short
#define u32 unsigned int

#define USHORT unsigned short
#define DWORD unsigned int


#pragma pack (1)

    typedef struct _AMS_TCPheader {
	us reserved;		// must be zero for now;
	u32 length;		//length in bytes
    } AMS_TCPheader;



/**
    The AMS header.
*/
    typedef struct _AMSheader {
	AMSNetID targetId;
	us targetPort;
	AMSNetID sourceId;
	us sourcePort;
	us commandId;
	us stateFlags;
	u32 dataLength;
	u32 errorCode;
	u32 invokeId;		//user-defined 32-bit field. Usually it is used to identify
	// a response as belonging to a certain request sent with the 
	// same value
    } AMSheader;

/**
 * \brief Ads Command Id for AmsHeader
 * Other commands are not defined or are used internally. Therefore the Command Id is only allowed to contain the above enumerated values!
 */
    enum nAdsCommandId {
	/**Invalid*/
	cmdADSinvalid = 0x0000,
	/**ADS Read Device Info*/
	cmdADSreadDevInfo = 0x0001,
	/**ADS Read*/
	cmdADSread = 0x0002,
	/**ADS Write*/
	cmdADSwrite = 0x0003,
	/**ADS Read State*/
	cmdADSreadState = 0x0004,
	/**ADS Write Control*/
	cmdADSwriteControl = 0x0005,
	/**ADS Add Device Notification*/
	cmdADSaddDeviceNotification = 0x0006,
	/**ADS Delete Device Notification*/
	cmdADSdeleteDeviceNotification = 0x0007,
	/**ADS Device Notification*/
	cmdADSdevNotify = 0x0008,
	/**ADS Read Write*/
	cmdADSreadWrite = 0x0009
    } AdsCommandId;
/*
    Definitions of bits in stateFlags:
*/
#define sfAMSresponse	0x001	// zero means request
#define sfAMScommand	0x004	// indicates data exchange via ADS command
#define sfAMSudp	0x400	// indicates transport over UDP (0 mens TCP)

#define maxDataLen 1524		// just equal to ethernet packet length

    typedef struct _ADSpacket {
	AMS_TCPheader adsHeader;
	AMSheader amsHeader;
	char data[maxDataLen];
    } ADSpacket;


    typedef struct _ADSdeviceInfo {
	u32 ADSerror;
	AdsVersion Version;
	char name[16];		// rely on 16 byte, not terminating #0
    } ADSdeviceInfo;

    typedef struct _ADSreadRequest {
	u32 indexGroup;		// index Group the data belongs to
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
	us ADSstate;
	us devState;
    } ADSstateResponse;

    typedef struct _ADSwriteControlRequest {
	us ADSstate;
	us devState;
	u32 length;
	uc data[];
    } ADSwriteControlRequest;


    typedef struct _ADSaddNotificationRequest {
	u32 indexGroup;		// index Group the data belongs to
	u32 indexOffset;	// offset within index Group 
	u32 length;		// length in bytes to be read
	u32 transmissionMode;	//
	u32 maxDelay;		// after this time the notification will be called up.
	// unit is 100ns
	u32 cycleTime;		// after this time the server teests whether values did change
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
	u32 writeLength;	// length in bytes of response
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

#define ADSDebugOpen 0x10
#define ADSDebugPacket 0x20
#define ADSDebugByte 0x40
#define ADSDebugAnalyze 0x80
#define ADSDebugPrintErrors 0x8
#define ADSDebugAll (ADSDebugOpen|ADSDebugPacket|ADSDebugByte|ADSDebugAnalyze|ADSDebugPrintErrors)
#define ADSDebugNone 0x0

#ifndef ADSDebug
#define ADSDebug ADSDebugNone
#endif


/* 
    This is a wrapper for the serial or network interface. 
*/
    typedef struct {
	_ADSOSserialType fd;	// some handle for the serial/network interface
	int error;		// Set when read/write errors occur. You will have to do something
	// specific to your OS to make transort work again.
	int timeout;		// Timeout in microseconds used in transort.
	char *name;		// this name is used in error outut, so you can identify the interface  
	AMSNetID me;
	int AMSport;
    } ADSInterface;

/* 
    This holds data for a connection;
*/
    typedef struct {
	ADSInterface *iface;	// pointer to used interface
	int AnswLen;		// length of last message
	int invokeID;		// packetNumber in transport layer
	void *dataPointer;	// pointer to result data, if present
	uc msgIn[maxDataLen];
	uc msgOut[maxDataLen];
	AMSNetID partner;
	int AMSport;
	int invokeId;
    } ADSConnection;

    void ads_debug (int type, const char *fmt, ...);
/** 
    This will setup a new interface structure from an initialized
    serial interface's handle and a name.
*/
    ADSInterface *ADSNewInterface(_ADSOSserialType nfd,
						   AMSNetID me, int port,
						   char *nname);

/** 
    This will setup a new connection structure using an initialized
    ADSInterface and PLC's MPI address.
*/
    ADSConnection *ADSNewConnection(ADSInterface * di,
						     AMSNetID partner,
						     int port);

/**
    Hex dump:
*/
    void _ADSDump(char *name, void *v, int len);


    void _ADSDumpAMSNetId(AMSNetID * id);

    void _ADSDumpAMSheader(AMSheader * h);

/**
    Naming
*/
    char *ADSCommandName(int c);

    char *ADSerrorText(int err);

    int _ADSReadOne(ADSInterface * di, uc * b);

    int _ADSReadPacket(ADSInterface * di, uc * b);

    void analyze(uc * p1);

    int ADSreadBytes(ADSConnection * dc, int indexGroup,
				      int offset, int length,
				      void *buffer);
    int ADSreadDeviceInfo(ADSConnection * dc,
					   char *pDevName,
					   PAdsVersion pVersion);
    int ADSwriteBytes(ADSConnection * dc, int indexGroup,
				       int offset, int length, void *data);
    int ADSreadState(ADSConnection * dc,
				      unsigned short *ADSstate,
				      unsigned short *devState);
    int ADSwriteControl(ADSConnection * dc, int ADSstate,
					 int devState, void *data,
					 int length);
    int ADSaddDeviceNotification(ADSConnection * dc,
						  int indexGroup,
						  int offset, int length,
						  int transmissionMode,
						  int maxDelay,
						  int cycleTime);
    int ADSreadWriteBytes(ADSConnection * dc,
					   int indexGroup, int offset,
					   int readLength,
					   void *readBuffer,
					   int writeLength,
					   void *writeBuffer);
    int _ADSwrite(ADSConnection * dc);

    int ADSparseNetID(const char *NetIDstring,
				       AMSNetID * id);

    ADSConnection *AdsSocketConnect(int *socket_fd, PAmsAddr pAddr,
				    PAmsAddr pMeAddr);

    int AdsSocketDisconnect(int *fd);

    int freeADSInterface(ADSInterface * di);

    int freeADSConnection(ADSConnection * dc);

#endif				/* __ADS */

#ifdef __cplusplus
//#ifdef CPLUSPLUS
}
#endif

