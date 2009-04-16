/*
 Implementation of BECKHOFF's ADS protocol. 
 Api Source File
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

#include "ads.h"
#include "AdsDEF.h"
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>
#include <net/if.h>

static int socket_fd = 0;

static ADSConnection *AdsSocketConnect(PAmsAddr pAddr, PAmsAddr pMeAddr);

/**
  * Establishes a connection (communication port) to the TwinCAT message router.
  * \return A port number that has been assigned to the program by the ADS router is returned. 
  */
int AdsPortOpen(void) {
    if (socket_fd == 0) {
		socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    }
    /*
	 * TODO: Should return a Port number. But ...
	 */
   return 0;
}


/**
 * The connection (communication port) to the TwinCAT message router is closed. 
 * \return Returns The function's error status. 
 */
long AdsPortClose(void) {
	if (socket_fd = 0) {
		return 0xD; /* Port not connected */
	}
	close(socket_fd);
	socket_fd = 0;
	return 0;
}

/**
  * Returns the local NetId and port number. 
  * \return Returns the function's error status. 
  * \param pAddr Pointer to the structure of type AmsAddr. 
  */
long AdsGetLocalAddress( PAmsAddr pAddr )  {
	struct ifaddrs *list;
	unsigned char b[4];
	unsigned long int netAddr;
	struct sockaddr_in *addrStruct;
	
	if(getifaddrs(&list) < 0)
	{	
		return 0;
	}
	
	struct ifaddrs *cur;	
	for(cur = list; cur != NULL; cur = cur->ifa_next)
	{
		if ((cur->ifa_addr->sa_family == AF_INET) && (strcmp(cur->ifa_name, "lo") != 0) ) {
			addrStruct = (struct sockaddr_in *)cur->ifa_addr;
			netAddr = ntohl (addrStruct->sin_addr.s_addr);
			memcpy ((char *) &b, (char *)&netAddr,4);
			pAddr->netId = (AmsNetId) {b[3], b[2], b[1], b[0], 1, 1};
			break;
		}
		if (cur->ifa_next == NULL)
			pAddr->netId = (AmsNetId) {127,0,0,1,1,1};
	}	
	freeifaddrs(list);

	return 0;
} 

/** \brief Changes the ADS status and the device status of an ADS server. 
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
long AdsSyncWriteControlReq( PAmsAddr pAddr,
							unsigned short nAdsState,
							unsigned short nDeviceState,
							unsigned long nLength,
							void *pData ) {
                                 
    ADSConnection *dc;
    dc = AdsSocketConnect(pAddr, NULL);                             
	ADSwriteControl(dc, nAdsState, nDeviceState, pData, nLength);

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
long AdsSyncWriteReq( PAmsAddr pAddr,
						unsigned short nIndexGroup,
						unsigned short nIndexOffset,
						unsigned long nLength,
						void *pData ) {
      
    ADSConnection *dc;
    dc = AdsSocketConnect(pAddr, NULL);    
 	ADSwriteBytes(dc, nIndexGroup, nIndexOffset, nLength, pData);

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
long AdsSyncReadReq( PAmsAddr pAddr,
						unsigned short nIndexGroup,
						unsigned short nIndexOffset,
						unsigned long nLength,
						void *pData ) {
      
    ADSConnection *dc;
    AmsAddr MeAddr;
    PAmsAddr pMeAddr;
    MeAddr.netId = (AmsNetId) {172,16,17,1,1,1};
    dc = AdsSocketConnect(pAddr, pMeAddr);    
 	ADSreadBytes(dc, nIndexGroup, nIndexOffset, nLength, pData);

	return 0;
}

/**
 * \brief Reads the ADS status and the device status from an ADS server.
 * Any ADS device can inform another ADS device of its current state. A distinction is drawn here between the status of the device itself (DeviceState) and the status of the ADS interface of the ADS device (AdsState). The states that the ADS interface can adopt are laid down in the ADS specification.
 * \param pAddr Structure with NetId and port number of the ADS server.
 * \param Address of a variable that will receive the ADS status (see data type ADSSTATE).
 * \param Address of a variable that will receive the device status. 
 * \return Returns the function's error status.
 *
 */
long AdsSyncReadStateReq(   PAmsAddr  pAddr, 
							unsigned short *pAdsState, 
							unsigned short *pDeviceState ){
	ADSConnection *dc;
    AmsAddr MeAddr;
    PAmsAddr pMeAddr;
    MeAddr.netId = (AmsNetId) {172,16,17,1,1,1};
    dc = AdsSocketConnect(pAddr, pMeAddr);    
 	ADSreadState(dc, pAdsState, pDeviceState);

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
static ADSConnection *AdsSocketConnect(PAmsAddr pAddr, PAmsAddr pMeAddr) {

	struct sockaddr_in addr;
    socklen_t addrlen;   
	char peer[12];
	int opt;
	ADSInterface *di;
	ADSConnection *dc;
	_ADSOSserialType fds;
	if (pMeAddr == NULL) {
		AmsAddr tempAddr;
		pMeAddr = &tempAddr;
		AdsGetLocalAddress( pMeAddr );
	}
	/*
	 * If there is no defined socket already open, exit with error.
	 */
	if (socket_fd == 0)
		return NULL;
	/* Build socket address */
	addr.sin_family = AF_INET;
    addr.sin_port = htons(0xBF02); /* ADS port 48898 */
	/* lazy convertion from byte array to socket adress format */
	sprintf (peer,"%d.%d.%d.%d", pAddr->netId.b1, pAddr->netId.b2, pAddr->netId.b3, pAddr->netId.b4);
	inet_aton(peer, &addr.sin_addr);
	
	addrlen = sizeof(addr);
	
	/* connect to plc */
    if (connect(socket_fd, (struct sockaddr *) & addr, addrlen)) {
		printf("Socket error: %s \n", 0000);
		return NULL;
    }
	if (ADSDebug & ADSDebugOpen) {
		printf ("connected to %s", peer);
	}  
	errno=0;
	opt=1;
	setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, 4);
	if (ADSDebug & ADSDebugOpen) {	
	    printf("setsockopt %s %d\n", strerror(errno),0);
	}  
	
    fds.rfd=socket_fd;
    fds.wfd=socket_fd;
    di=ADSNewInterface(fds,pMeAddr->netId, pAddr->port,"test");
	dc=ADSNewConnection(di,pAddr->netId, pAddr->port);
	
	return dc;
}

