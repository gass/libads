/*
 Implementation of BECKHOFF's ADS protocol.
 Definitions Header.
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System Nov 2002.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of BECKHOFF 
 Company. www.beckhoff.de

 Copyright (C) Luis Matos (gass@otiliamatos.ath.cx) 2009.

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
#ifndef __ADSDEF_H__
#define __ADSDEF_H__

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
#define ADSPortCamshaftController			900

/*
 * ADS index groups
 */

#define ADSIGRP_SYMTAB					0xF000
#define ADSIGRP_SYMNAME					0xF001
#define ADSIGRP_SYMVAL					0xF002

#define ADSIGRP_SYM_HNDBYNAME				0xF003
#define ADSIGRP_SYM_VALBYNAME				0xF004
#define ADSIGRP_SYM_VALBYHND				0xF005
#define ADSIGRP_SYM_RELEASEHND				0xF006
#define ADSIGRP_SYM_INFOBYNAME				0xF007
#define ADSIGRP_SYM_VERSION					0xF008
#define ADSIGRP_SYM_INFOBYNAMEEX			0xF009

#define ADSIGRP_SYM_DOWNLOAD				0xF00A
#define ADSIGRP_SYM_UPLOAD					0xF00B
#define ADSIGRP_SYM_UPLOADINFO				0xF00C

#define ADSIGRP_SYMNOTE						0xF010		// notification of named handle

#define ADSIGRP_IOIMAGE_RWIB				0xF020		// read/write input byte(s)
#define ADSIGRP_IOIMAGE_RWIX				0xF021		// read/write input bit
#define ADSIGRP_IOIMAGE_RWOB				0xF030		// read/write output byte(s)
#define ADSIGRP_IOIMAGE_RWOX				0xF031		// read/write output bit
#define ADSIGRP_IOIMAGE_CLEARI				0xF040		// write inputs to null
#define ADSIGRP_IOIMAGE_CLEARO				0xF050		// write outputs to null

#define ADSIGRP_DEVICE_DATA					0xF100		// state, name, etc...
#define ADSIOFFS_DEVDATA_ADSSTATE			0x0000		// ads state of device
#define ADSIOFFS_DEVDATA_DEVSTATE			0x0002		// device state


/* TypeDef and structure definition */

/**
    \brief  The NetId of and ADS device can be represented in this structure.

    According to BECKHOFF, AMS Id is a six byte field. In case of IP transport, the first
    4 bytes are identical to the IP Address.
*/
typedef struct _AMSNetID {
    unsigned char b1,b2,b3,b4;	// < normally equal to IP address.
    unsigned char  b5;		// < extra byte defined as 1.
    unsigned char  b6;		// < extra byte defined as 1.
} AMSNetID, AmsNetId;

/**
 * The complete address of an ADS device can be stored in this structure.
 */
typedef struct _AmsAddr {
    AmsNetId netId;
    unsigned short port;	// < ADS Port number.
} AmsAddr, * PAmsAddr;

/**
 * The structure contains the version number, revision number and build number.
 * TODO: not used
 */
typedef	struct 
{
	unsigned char		version; // < Version number.
	unsigned char		revision; // < Revision number.
	unsigned short		build; // < Build number
} AdsVersion;

typedef	AdsVersion*	PAdsVersion;

/**
  *
  */
typedef enum nAdsTransMode
{
	ADSTRANS_NOTRANS		=0,
	ADSTRANS_CLIENTCYCLE	=1,
	ADSTRANS_CLIENT1REQ		=2,
	ADSTRANS_SERVERCYCLE	=3,
	ADSTRANS_SERVERONCHA	=4
}ADSTRANSMODE;

typedef enum nAdsState
{
	ADSSTATE_INVALID			=0,
	ADSSTATE_IDLE				=1,
	ADSSTATE_RESET				=2,
	ADSSTATE_INIT				=3,
	ADSSTATE_START				=4,
	ADSSTATE_RUN				=5,
	ADSSTATE_STOP				=6,
	ADSSTATE_SAVECFG			=7,
	ADSSTATE_LOADCFG			=8,
	ADSSTATE_POWERFAILURE		=9,
	ADSSTATE_POWERGOOD			=10,
	ADSSTATE_ERROR				=11,
	ADSSTATE_SHUTDOWN			=12,
	ADSSTATE_SUSPEND			=13,
	ADSSTATE_RESUME				=14,
	ADSSTATE_MAXSTATES
} ADSSTATE;

#endif	// __ADSDEF_H__
