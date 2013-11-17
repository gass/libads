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

#include <stdio.h>

#include "ads.h"
#include "AdsDEF.h"
#include "ads_connect.h"
#include "AdsAPI.h"
#include "debugprint.h"

extern ADSConnection 	**pADSConnectionList;
extern int				nADSConnectionCnt;
int defaultPort = 0;	//port used by "not extended" functions,
						//set to 1 by AdsPortOpen()


/**
 * @brief Simply returns the version of this api
 * @return A long int that must be converted to an AdsVersion structure
 * is returned.
 */
long AdsGetDllVersion( void )
{
	union {
		AdsVersion ver;
		long num;
	} v;
	v.ver.version = 0;
	v.ver.revision = 1;
	v.ver.build = 0;

	return v.num;
}

/**
 * @brief Establishes a connection (communication port) to the TwinCAT
 * @brief message router. (Beckhoff says)
 * As we dont use a message router, we do nothing here...
 * Hint: the opening of a connection takes place at the first read or write
 * operation to/from a PLC. ADSsocketConnect() does it.
 * @return 0x0 (ADS Success)
 */
long AdsPortOpen(void)
{
	defaultPort = 1;
	return defaultPort;
}

/**
 * @brief The connection (communication port) to the TwinCAT message router
 * is closed. (Beckhoff says)
 * As we dont use a message router, we close all open sockets
 * (= connections to PLC's)
 * @return Returns The function's error status.
 */
long AdsPortClose(void)
{
	int i;

	for(i = 0; i < nADSConnectionCnt; i++){
		ADSsocketDisconnect(pADSConnectionList[i]);
		ADSFreeConnection(pADSConnectionList[i]);
	}
	free(pADSConnectionList);
	nADSConnectionCnt = 0;
	return 0;
}

/**
 * @brief Returns the local NetId and port number.
 * @param port  port number of an Ads port that had previously been opened with
 *				AdsPortOpenEx or AdsPortOpen..
 * @param pAddr Pointer to the structure of type AmsAddr.
 * @return Returns the function's error status.
 */
long AdsGetLocalAddressEx(long port, PAmsAddr pAddr)
{
	if(port <= 0){
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR,
			   "AdsGetMeAddress(): returns 0x18, port %d not valid.", port);
#endif
		MsgOut(MSG_ERROR,
			   MsgStr("AdsGetMeAddress(): returns 0x18, port %d not valid.\n",
					  port));
		return(0x18);
	}

	return AdsGetMeAddress(pAddr, AMSPORT_R0_PLC_RTS1);
}

/**
 * @brief A frontend to AdsGetLocalAddressEx() with port = defaultPort
 */
long AdsGetLocalAddress(PAmsAddr pAddr)
{
	return AdsGetLocalAddressEx(defaultPort, pAddr);
}

/** @brief Changes the ADS status and the device status of an ADS server.
  *
  * In addition to changing the ADS status and the device status, it is also
  * possible to send data to the ADS server in order to transfer further
  * information. In the current ADS devices (PLC, NC, ...) this data has no
  * further effect.
  * Any ADS device can inform another ADS device of its current state.
  * A distinction is drawn here between the status of the device itself
  * (DeviceState) and the status of the ADS interface of the ADS device
  * (AdsState).
  * The states that the ADS interface can adopt are laid down in the ADS
  * specification.
  * @param port port number of an Ads port that had previously been opened with
  *				AdsPortOpenEx or AdsPortOpen..
  * @param pAddr Structure with NetId and port number of the ADS server.
  * @param nAdsState New ADS status.
  * @param nDeviceState New device status.
  * @param nLength Length of the data in bytes.
  * @param pData Pointer to data sent additionally to the ADS device.
  * @return Returns the function's error status.
  */
long AdsSyncWriteControlReqEx(long port, PAmsAddr pAddr,
			    unsigned short nAdsState,
			    unsigned short nDeviceState,
			    unsigned long nLength, void *pData)
{
    ADSConnection *dc;
	int adsError, ret;

	dc = ADSsocketGet(port, pAddr, &adsError);
	if(!dc)
		return adsError;

	ret = ADSwriteControl(dc, nAdsState, nDeviceState, pData, nLength);
	return(ret);

}

/**
 * @brief A frontend to AdsSyncWriteControlReqEx() with port = defaultPort
 */
long AdsSyncWriteControlReq(PAmsAddr pAddr,
							  unsigned short nAdsState,
							  unsigned short nDeviceState,
							  unsigned long nLength, void *pData)
{
	return(AdsSyncWriteControlReqEx(defaultPort, pAddr,
		   							nAdsState, nDeviceState,
		   							nLength, pData));
}

/**
 * @brief Writes data synchronously to an ADS device.
 * @param port  port number of an Ads port that had previously been opened with
 *				AdsPortOpenEx or AdsPortOpen..
 * @param pAddr Structure with NetId and port number of the ADS server.
 * @param nIndexGroup Index Group.
 * @param nIndexOffset Index Offset.
 * @param nLength Length of the data, in bytes, written to the ADS server.
 * @param pData Pointer to the data written to the ADS server.
 * @return Returns the function's error status.
 */
long AdsSyncWriteReqEx(long port, PAmsAddr pAddr,
		     unsigned long nIndexGroup,unsigned long nIndexOffset,
		     unsigned long nLength, void *pData)
{
    ADSConnection *dc;
	int adsError, ret;

	dc = ADSsocketGet(port, pAddr, &adsError);
	if(!dc)
		return adsError;

	ret = ADSwriteBytes(dc, nIndexGroup, nIndexOffset, nLength, pData);

	return(ret);
}

/**
 * @brief A frontend to AdsSyncWriteReqEx() with port = defaultPort
 */
long AdsSyncWriteReq( PAmsAddr pAddr,
					   unsigned long nIndexGroup,unsigned long nIndexOffset,
					   unsigned long nLength, void *pData)
{
	return(AdsSyncWriteReqEx(defaultPort, pAddr,
		   nIndexGroup, nIndexOffset,
		   nLength, pData));
}

/**
 * @brief Reads data synchronously from an ADS server.
 * @param port  port number of an Ads port that had previously been opened with
 *				AdsPortOpenEx or AdsPortOpen..
 * @param pAddr Structure with NetId and port number of the ADS server.
 * @param nIndexGroup Index Group.
 * @param nIndexOffset Index Offset.
 * @param nLength Length of the data, in bytes, written to the ADS server.
 * @param pData Pointer to the data written to the ADS server.
 * @param pnRead variable that returns the number of succesfully read
 *                  data bytes.
 * @return Returns the function's error status.
 */
long AdsSyncReadReqEx2(long port, PAmsAddr pAddr,
					unsigned long nIndexGroup, unsigned long nIndexOffset,
					unsigned long nLength, void *pData,
					unsigned long *pnRead)
{
	ADSConnection *dc;
	int adsError;

	dc = ADSsocketGet(port, pAddr, &adsError);
	if(!dc)
		return adsError;

	adsError = ADSreadBytes(dc, nIndexGroup, nIndexOffset, nLength, pData, pnRead);

	return adsError;
}

/**
 * @brief A frontend to AdsSyncReadReqEx2() with port = defaultPort
 */
long AdsSyncReadReqEx(PAmsAddr pAddr,
					unsigned long nIndexGroup,
					unsigned long nIndexOffset,
					unsigned long nLength, void *pData,
					unsigned long *pnRead)
{
	return AdsSyncReadReqEx2(defaultPort, pAddr,
							 nIndexGroup, nIndexOffset,
							 nLength, pData,
							 pnRead);
}

/**
 * @brief A frontend to AdsSyncReadReqEx2() with port = defaultPort and pnRead = NULL
 */
long AdsSyncReadReq(PAmsAddr pAddr,
					unsigned long nIndexGroup,
					unsigned long nIndexOffset,
					unsigned long nLength, void *pData)
{
	return AdsSyncReadReqEx2(defaultPort, pAddr,
							 nIndexGroup, nIndexOffset,
							 nLength, pData,
							 NULL);
}

/**
 * @brief Reads the ADS status and the device status from an ADS server.
 * Any ADS device can inform another ADS device of its current state.
 * A distinction is drawn here between the status of the device itself
 * (DeviceState) and the status of the ADS interface of the ADS device
 * (AdsState). The states that the ADS interface can adopt are laid down
 * in the ADS specification.
 * @param port  port number of an Ads port that had previously been opened with
 *				AdsPortOpenEx or AdsPortOpen..
 * @param pAddr Structure with NetId and port number of the ADS server.
 * @param pAdsState Address of a variable that will receive the ADS status
 *					(see data type ADSSTATE).
 * @param pDeviceState Address of a variable that will receive the device status.
 * @return Returns the function's error status.
 *
 */
long AdsSyncReadStateReqEx(long port, PAmsAddr pAddr,
			 unsigned short *pAdsState,
			 unsigned short *pDeviceState)
{
	ADSConnection *dc;

	int adsError;

	dc = ADSsocketGet(port, pAddr, &adsError);
	// memory for dc is allocated by malloc() within  _ADSnewConnection()
	if(!dc)
		return adsError;

	adsError = ADSreadState(dc, pAdsState, pDeviceState);

	return adsError;
}

/**
 * @brief A frontend to AdsSyncReadStateReqEx() with port = defaultPort
 */
long AdsSyncReadStateReq(PAmsAddr pAddr,
						 unsigned short *pAdsState,
						 unsigned short *pDeviceState)
{

	return AdsSyncReadStateReqEx(defaultPort, pAddr, pAdsState, pDeviceState);
}

/**
 * Reads the identification and version number of an ADS server.
 * @param port  port number of an Ads port that had previously been opened with
 *				AdsPortOpenEx or AdsPortOpen..
 * @param pAddr Structure with NetId and port number of the ADS server.
 * @param pDevName Pointer to a character string that will receive the name of
 *		the ADS device.
 * @param pVersion Address of a variable of type AdsVersion, which will receive
 *		the version number, revision number and the build number.
 * @return Returns the function's error status.
 */
long AdsSyncReadDeviceInfoReqEx(long port, PAmsAddr pAddr,
							  	char *pDevName,
							  	PAdsVersion pVersion)
{
    ADSConnection *dc;
	int adsError;

	dc = ADSsocketGet(port, pAddr, &adsError);
	if(!dc)
		return adsError;

	adsError = ADSreadDeviceInfo(dc, pDevName, pVersion);
	return adsError;
}

/**
 * @brief A frontend to AdsSyncReadDeviceInfoReqEx() with port = defaultPort
 */
long AdsSyncReadDeviceInfoReq(PAmsAddr pAddr,
							  char *pDevName,
							  PAdsVersion pVersion)
{
	return(AdsSyncReadDeviceInfoReqEx(defaultPort, pAddr,
		   								pDevName, pVersion));
}

/**
 * @brief Writes data synchronously into an ADS server and receives data back
 * @brief from the ADS device.
 * @param port  port number of an Ads port that had previously been opened with
 *				AdsPortOpenEx or AdsPortOpen..
 * @param pAddr Structure with NetId and port number of the ADS server.
 * @param nIndexGroup Index Group.
 * @param nIndexOffset Index Offset.
 * @param nReadLength Length of the data, in bytes, returned by the ADS device.
 * @param pReadData Buffer with data returned by the ADS device.
 * @param nWriteLength Length of the data, in bytes, written to the ADS server.
 * @param pWriteData Buffer with data written to the ADS device.
 * @param pcbReturn If successful, this variable will return the number of
 *					actually read data bytes
 * @return Returns the function's error status.
 */
long AdsSyncReadWriteReqEx2(long port, PAmsAddr pAddr,
						 unsigned long nIndexGroup, unsigned long nIndexOffset,
						 unsigned long nReadLength, void *pReadData,
						 unsigned long nWriteLength, void *pWriteData,
						 unsigned long *pcbReturn)
{
    ADSConnection *dc;
	int adsError;

	dc = ADSsocketGet(port, pAddr, &adsError);
	if(!dc)
		return adsError;

	adsError = ADSreadWriteBytes(dc,
								 nIndexGroup, nIndexOffset,
								 nReadLength, pReadData,
								 nWriteLength, pWriteData,
								 pcbReturn
								);
	return adsError;
}

/**
 * @brief A frontend to AdsSyncReadWriteReqEx2() with port = defaultPort
 */
long AdsSyncReadWriteReqEx(PAmsAddr pAddr,
						 unsigned long nIndexGroup,
						 unsigned long nIndexOffset,
						 unsigned long nReadLength,
						 void *pReadData,
						 unsigned long nWriteLength, void *pWriteData,
						 unsigned long *pnRead)
{
	return AdsSyncReadWriteReqEx2(defaultPort, pAddr,
								 nIndexGroup, nIndexOffset,
								 nReadLength, pReadData,
								 nWriteLength, pWriteData,
								 pnRead);
}

/**
 * @brief A frontend to AdsSyncReadWriteReqEx2() with port = defaultPort and pnRead = NULL
 */
long AdsSyncReadWriteReq(PAmsAddr pAddr,
						 unsigned long nIndexGroup,
						 unsigned long nIndexOffset,
						 unsigned long nReadLength,
						 void *pReadData,
						 unsigned long nWriteLength, void *pWriteData)
{
	return AdsSyncReadWriteReqEx2(defaultPort, pAddr,
								 nIndexGroup, nIndexOffset,
								 nReadLength, pReadData,
								 nWriteLength, pWriteData,
								 NULL);
}

/**
 * @brief Alters the timeout for the ADS functions. The standard value is 5000 ms.
 * @param port port number of an Ads port that had previously been opened with
 * 			   AdsPortOpenEx or AdsPortOpen.
 * @param nMs Timeout in ms.
 * @return the function's error status.
 */
long AdsSyncSetTimeoutEx(long port, long nMs)
{
	return AdsSetTimeout(port, nMs);
}

/**
 * @brief A frontend to AdsSyncSetTimeoutEx() with port = defaultPort
 */
long AdsSyncSetTimeout(long nMs)
{
	return (AdsSyncSetTimeoutEx(defaultPort, nMs));
}

/**
 * A helper function to convert a Windows Filetime (64 bit)
 * to an UNIX time
 * @param winTime Windows Filetime
 * @return Returns the equivalent UNIX time.
 */
#define EPOCH_DIFF 116444736000000000LL

time_t FileTime2tt(long long winTime)
{
	time_t tt;

	winTime -= EPOCH_DIFF;
	winTime /= 10000000LL;
	tt = (time_t)winTime;

	return tt;
}
