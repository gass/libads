/*
 Implementation of BECKHOFF's ADS protocol.
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System
 May 2011.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of
 BECKHOFF Company. www.beckhoff.de

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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ADSDEF_H__
#define __ADSDEF_H__

#include <stdint.h>

#ifndef	ANYSIZE_ARRAY
#define	ANYSIZE_ARRAY	1
#endif

/*
 * ADS Available Ports
 */

#define	AMSPORT_LOGGER					100
#define	AMSPORT_R0_RTIME				200
#define	AMSPORT_R0_TRACE				(AMSPORT_R0_RTIME+90)
#define	AMSPORT_R0_IO					300
#define	AMSPORT_R0_SPS					400
#define	AMSPORT_R0_NC					500
#define	AMSPORT_R0_ISG					550
#define	AMSPORT_R0_PCS					600
#define	AMSPORT_R0_PLC					801
#define	AMSPORT_R0_PLC_RTS1				801
#define	AMSPORT_R0_PLC_RTS2				811
#define	AMSPORT_R0_PLC_RTS3				821
#define	AMSPORT_R0_PLC_RTS4				831
#define ADSPortCamshaftController		900

/*
 * ADS index groups
 */
#define ADSIGRP_IOIMAGE_FLAGS			0x4020	//PLC memory area ( AT %MB...)
#define ADSIGRP_IOIMAGE_FLAGBITS		0x4021	//PLC memory area - bits ( AT %MX...)
#define ADSIGRP_IOIMAGE_DATA			0x4040  //PLC data area

#define ADSIGRP_SYMTAB					0xF000
#define ADSIGRP_SYMNAME					0xF001
#define ADSIGRP_SYMVAL					0xF002
#define ADSIGRP_SYM_HNDBYNAME			0xF003
#define ADSIGRP_SYM_VALBYNAME			0xF004
#define ADSIGRP_SYM_VALBYHND			0xF005
#define ADSIGRP_SYM_RELEASEHND			0xF006

#define ADSIGRP_SYM_INFOBYNAME			0xF007
#define ADSIGRP_SYM_VERSION				0xF008
#define ADSIGRP_SYM_INFOBYNAMEEX		0xF009
#define ADSIGRP_SYM_DOWNLOAD			0xF00A
#define ADSIGRP_SYM_UPLOAD				0xF00B	// read declared symbols (variables) from PLC
#define ADSIGRP_SYM_UPLOADINFO			0xF00C	// read count and sizes for symbols from PLC
#define ADSIGRP_SYM_DT_UPLOAD			0xF00E	// read datatye-descriptions  from PLC
#define ADSIGRP_SYM_UPLOADINFO2			0xF00F	// read count and sizes for symbols and datatypes from PLC

#define ADSIGRP_SYMNOTE					0xF010	//notification of named handle
#define ADSIGRP_IOIMAGE_RWIB			0xF020	//read/write input byte(s)
#define ADSIGRP_IOIMAGE_RWIX			0xF021	//read/write input bit
#define ADSIGRP_IOIMAGE_RWOB			0xF030	//read/write output byte(s)
#define ADSIGRP_IOIMAGE_RWOX			0xF031	//read/write output bit
#define ADSIGRP_IOIMAGE_CLEARI			0xF040	//write inputs to null
#define ADSIGRP_IOIMAGE_CLEARO			0xF050	//write outputs to null
#define ADSIGRP_SUMUP_READ				0xF080 	//read a list of variables with one single ADS-command
#define ADSIGRP_SUMUP_WRITE				0xF081	//write a list of variables with one single ADS-command
#define ADSIGRP_DEVICE_DATA				0xF100	//state, name, etc...

/*
 * state flags
 */
#define ADSIOFFS_DEVDATA_ADSSTATE		0x0000	//ads state of device
#define ADSIOFFS_DEVDATA_DEVSTATE		0x0002	//device state

#define ADSERR_NOERR 					0x0

#pragma pack(push)
#pragma pack(1)

typedef enum nAdsTransMode
{
	ADSTRANS_NOTRANS     = 0,
	ADSTRANS_CLIENTCYCLE = 1,
	ADSTRANS_CLIENT1REQ  = 2,
	ADSTRANS_SERVERCYCLE = 3,
	ADSTRANS_SERVERONCHA = 4
} ADSTRANSMODE;

typedef enum nAdsState
{
	ADSSTATE_INVALID 		= 0,
	ADSSTATE_IDLE 			= 1,
	ADSSTATE_RESET 			= 2,
	ADSSTATE_INIT 			= 3,
	ADSSTATE_START 			= 4,
	ADSSTATE_RUN 			= 5,
	ADSSTATE_STOP 			= 6,
	ADSSTATE_SAVECFG 		= 7,
	ADSSTATE_LOADCFG 		= 8,
	ADSSTATE_POWERFAILURE 	= 9,
	ADSSTATE_POWERGOOD 		= 10,
	ADSSTATE_ERROR 			= 11,
	ADSSTATE_SHUTDOWN 		= 12,
	ADSSTATE_SUSPEND 		= 13,
	ADSSTATE_RESUME 		= 14,
	ADSSTATE_CONFIG		   	= 15, // system is in config mode
	ADSSTATE_RECONFIG		= 16, // system should restart in config mode
	ADSSTATE_MAXSTATES
} ADSSTATE;

/**
  *	\brief  The NetId of and ADS device can be represented in this structure.
  * According to BECKHOFF, AMS Id is a six byte field.
  * In our case, the first 4 bytes MUST be identical to the IP Address.
 */
typedef struct _AMSNetId
{
	unsigned char b[6];
} AmsNetId, *PAmsNetId;

/**
 * The complete address of an ADS device can be stored in this structure.
 */
typedef struct _AmsAddr {
	AmsNetId netId;
	unsigned short port;	// < ADS Port number.
} AmsAddr, *PAmsAddr;

/**
 * The structure contains the version number, revision number and build number.
 */
typedef struct {
	unsigned char version;	// Version number.
	unsigned char revision;	// Revision number.
	unsigned short build;	// Build number
} AdsVersion;
typedef AdsVersion *PAdsVersion;

typedef struct {
	unsigned long cbLength;
	ADSTRANSMODE nTransMode;
	unsigned long nMaxDelay;
	union {
		unsigned long nCycleTime;
		unsigned long dwChangeFilter;
	};
} AdsNotificationAttrib, *PAdsNotificationAttrib;

typedef struct {
	unsigned long hNotification;
	unsigned long long nTimeStamp;
	unsigned long cbSampleSize;
	unsigned char data[ANYSIZE_ARRAY];
} AdsNotificationHeader, *PAdsNotificationHeader;

typedef void (*PAdsNotificationFunc) (AmsAddr * Addr,
				      AdsNotificationHeader *
				      pNotification, unsigned long hUser);

typedef struct
{
        uint32_t    nSymbols;
        uint32_t   nSymSize;
} AdsSymbolUploadInfo, *PAdsSymbolUploadInfo;

////////////////////////////////////////////////////////////////////////////////
#define ADSSYMBOLFLAG_PERSISTENT                0x00000001
#define ADSSYMBOLFLAG_BITVALUE                  0x00000002

typedef struct
{
        uint32_t            entryLength;    // length of complete symbol entry
        uint32_t            iGroup;                 // indexGroup of symbol: input, output etc.
        uint32_t            iOffs;                  // indexOffset of symbol
        uint32_t            size;                           // size of symbol ( in bytes, 0 = bit )
        uint32_t            dataType;               // adsDataType of symbol
        uint32_t            flags;                  // see above
        uint16_t           nameLength;             // length of symbol name (excl. \0)
        uint16_t           typeLength;             // length of type name (excl. \0)
        uint16_t           commentLength;  // length of comment (excl. \0)
        // ADS_INT8             name[];                 // name of symbol with terminating \0
        // ADS_INT8             type[];                 // type name of symbol with terminating \0
        // ADS_INT8             comment[];              // comment of symbol with terminating \0
} AdsSymbolEntry, *PAdsSymbolEntry, **PPAdsSymbolEntry;


#define PADSSYMBOLNAME(p)                       ((char*)(((PAdsSymbolEntry)p)+1))
#define PADSSYMBOLTYPE(p)                       (((char*)(((PAdsSymbolEntry)p)+1))+((PAdsSymbolEntry)p)->nameLength+1)
#define PADSSYMBOLCOMMENT(p)            (((char*)(((PAdsSymbolEntry)p)+1))+((PAdsSymbolEntry)p)->nameLength+1+((PAdsSymbolEntry)p)->typeLength+1)

#define PADSNEXTSYMBOLENTRY(pEntry)     (*((uint32_t*)(((char*)pEntry)+((PAdsSymbolEntry)pEntry)->entryLength)) \
                                                ? ((PAdsSymbolEntry)(((char*)pEntry)+((PAdsSymbolEntry)pEntry)->entryLength)): NULL)
#pragma pack(pop)

#endif	// __ADSDEF_H__

#ifdef __cplusplus
}
#endif
