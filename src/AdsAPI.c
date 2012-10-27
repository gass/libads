/*
 Implementation of BECKHOFF's ADS protocol. 
 Api Source File
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System Nov 2002.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of BECKHOFF 
 Company. www.beckhoff.de

 Copyright (C) Luis Matos (gass@otiliamatos.ath.cx) 2009, 2012

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

#include <stdio.h>

#include "ads.h"
#include "AdsDEF.h"
#include "AdsAPI.h"

static int socket_fd = 0;
static AmsAddr MeAddr;

/**
  * \brief Establishes a connection (communication port) to the TwinCAT message router.
  * For now this is a dummy function
  * \return A port number that has been assigned to the program by the ADS router is returned. 
  */
int AdsPortOpen(void)
{
	PAmsAddr pMeAddr = &MeAddr;
	return AdsGetLocalAddress(pMeAddr);
}

/**
 * The connection (communication port) to the TwinCAT message router is closed. 
 * \return Returns The function's error status. 
 */
long AdsPortClose(void)
{

	return 0;
}

/**
  * Returns the local NetId and port number. 
  * \return Returns the function's error status. 
  * \param pAddr Pointer to the structure of type AmsAddr. 
  */
long AdsGetLocalAddress(PAmsAddr pAddr)
{
	return ADSGetLocalAMSId(&pAddr->netId);
}

/** \brief Changes the ADS status and the device status of an ADS server.
  *
  * In addition to changing the ADS status and the device status, it is also possible
  * to send data to the ADS server in order to transfer further information.
  * In the current ADS devices (PLC, NC, ...) this data has no further effect.
  * Any ADS device can inform another ADS device of its current state.
  * A distinction is drawn here between the status of the device itself (DeviceState)
  * and the status of the ADS interface of the ADS device (AdsState).
  * The states that the ADS interface can adopt are laid down in the ADS specification. 
  * \param pAddr Structure with NetId and port number of the ADS server.
  * \param nAdsState New ADS status. 
  * \param nDeviceState New device status. 
  * \param nLength Length of the data in bytes. 
  * \param pData Pointer to data sent additionally to the ADS device. 
  * \return Returns the function's error status. 
  */
long
AdsSyncWriteControlReq(PAmsAddr pAddr,
		       unsigned short nAdsState,
		       unsigned short nDeviceState,
		       unsigned long nLength, void *pData)
{

	ADSConnection *dc;
	dc = AdsSocketConnect(&socket_fd, pAddr, &MeAddr);
	if (dc == NULL)
		return 0x01;
	ADSwriteControl(dc, nAdsState, nDeviceState, pData, nLength);
	AdsSocketDisconnect(&socket_fd);
	freeADSConnection(dc);

	return 0;
}

/**
  * Writes data synchronously to an ADS device.
  * \param pAddr Structure with NetId and port number of the ADS server.
  * \param nIndexGroup Index Group.
  * \param nIndexOffset Index Offset.
  * \param nLength Length of the data, in bytes, written to the ADS server.
  * \param pData Pointer to the data written to the ADS server. 
  * \return Returns the function's error status.
  */
long
AdsSyncWriteReq(PAmsAddr pAddr,
		unsigned long nIndexGroup,
		unsigned long nIndexOffset, unsigned long nLength, void *pData)
{

	ADSConnection *dc;
	dc = AdsSocketConnect(&socket_fd, pAddr, &MeAddr);
	if (dc == NULL)
		return 0x01;
	ADSwriteBytes(dc, nIndexGroup, nIndexOffset, nLength, pData);
	AdsSocketDisconnect(&socket_fd);
	freeADSConnection(dc);

	return 0;
}

/**
 * Reads data synchronously from an ADS server.
 * \param pAddr Structure with NetId and port number of the ADS server.
 * \param nIndexGroup Index Group.
 * \param nIndexOffset Index Offset.
 * \param nLength Length of the data, in bytes, written to the ADS server.
 * \param pData Pointer to the data written to the ADS server. 
 * \return Returns the function's error status.
 */
long
AdsSyncReadReq(PAmsAddr pAddr,
	       unsigned long nIndexGroup,
	       unsigned long nIndexOffset, unsigned long nLength, void *pData)
{

	ADSConnection *dc;
	dc = AdsSocketConnect(&socket_fd, pAddr, &MeAddr);
	if (dc == NULL)
		return 0x01;
	socket_fd = dc->iface->fd.rfd;
	ADSreadBytes(dc, nIndexGroup, nIndexOffset, nLength, pData);
	AdsSocketDisconnect(&socket_fd);
	freeADSConnection(dc);

	return 0;
}

/**
 * \brief Reads the ADS status and the device status from an ADS server.
 * Any ADS device can inform another ADS device of its current state. A distinction is drawn here between the status of the device itself (DeviceState) and the status of the ADS interface of the ADS device (AdsState). The states that the ADS interface can adopt are laid down in the ADS specification.
 * \param pAddr Structure with NetId and port number of the ADS server.
 * \param pAdsState Address of a variable that will receive the ADS status (see data type ADSSTATE).
 * \param pDeviceState Address of a variable that will receive the device status. 
 * \return Returns the function's error status.
 *
 */
long
AdsSyncReadStateReq(PAmsAddr pAddr,
		    unsigned short *pAdsState, unsigned short *pDeviceState)
{
	ADSConnection *dc;
	dc = AdsSocketConnect(&socket_fd, pAddr, &MeAddr);
	if (dc == NULL)
		return 0x01;
	ADSreadState(dc, pAdsState, pDeviceState);
	AdsSocketDisconnect(&socket_fd);
	freeADSConnection(dc);

	return 0;
}

/**
 * Reads the identification and version number of an ADS server.
 * \param pAddr Structure with NetId and port number of the ADS server.
 * \param pDevName Pointer to a character string that will receive the name of the ADS device.
 * \param pVersion Address of a variable of type AdsVersion, which will receive the version number, revision number and the build number.
 * \return Returns the function's error status.
 */
long
AdsSyncReadDeviceInfoReq(PAmsAddr pAddr, char *pDevName, PAdsVersion pVersion)
{

	ADSConnection *dc;
	dc = AdsSocketConnect(&socket_fd, pAddr, &MeAddr);
	if (dc == NULL)
		return 0x01;
	ADSreadDeviceInfo(dc, pDevName, pVersion);
	AdsSocketDisconnect(&socket_fd);
	freeADSConnection(dc);

	return 0;
}

/**
 * Reads the identification and version number of an ADS server.
 * \param pAddr Structure with NetId and port number of the ADS server.
 * \param nIndexGroup Index Group.
 * \param nIndexOffset Index Offset.
 * \param nReadLength Length of the data, in bytes, returned by the ADS device.
 * \param pReadData Buffer with data returned by the ADS device.
 * \param nWriteLength Length of the data, in bytes, written to the ADS server.
 * \param pWriteData Buffer with data written to the ADS device.
 * \return Returns the function's error status.
 */
long
AdsSyncReadWriteReq(PAmsAddr pAddr,
		    unsigned long nIndexGroup,
		    unsigned long nIndexOffset,
		    unsigned long nReadLength,
		    void *pReadData,
		    unsigned long nWriteLength, void *pWriteData)
{
	ADSConnection *dc;
	dc = AdsSocketConnect(&socket_fd, pAddr, &MeAddr);
	if (dc == NULL)
		return 0x01;
	ADSreadWriteBytes(dc, nIndexGroup, nIndexOffset, nReadLength,
			  pReadData, nWriteLength, pWriteData);
	AdsSocketDisconnect(&socket_fd);
	freeADSConnection(dc);

	return 0;
}

/**
 * A notification is defined within an ADS server (e.g. PLC). When a certain event occurs a function (the callback
function) is invoked in the ADS client (C program).
 * \param pAddr Structure with NetId and port number of the ADS server.
 * \param nIndexGroup Index Group.
 * \param nIndexOffset Index Offset.
 * \param pNoteAttrib Pointer to the structure that contains further information.
 * \param pNoteFunc Name of the callback function.
 * \param hUser 32-bit value that is passed to the callback function.
 * \param pNotification Address of the variable that will receive the handle of the notification.
 * \return Returns the function's error status.
 */
long
AdsSyncAddDeviceNotificationReq(PAmsAddr pAddr,
				unsigned long nIndexGroup,
				unsigned long nIndexOffset,
				PAdsNotificationAttrib pNoteAttrib,
				PAdsNotificationFunc pNoteFunc,
				unsigned long hUser,
				unsigned long *pNotification)
{
	ADSConnection *dc;
	dc = AdsSocketConnect(&socket_fd, pAddr, &MeAddr);
	if (dc == NULL)
		return 0x01;
	ADSaddDeviceNotification(dc, nIndexGroup, nIndexOffset,
				 pNoteAttrib->cbLength,
				 pNoteAttrib->nTransMode,
				 pNoteAttrib->nMaxDelay,
				 pNoteAttrib->nCycleTime);
	AdsSocketDisconnect(&socket_fd);
	freeADSConnection(dc);

	return 0;
}
