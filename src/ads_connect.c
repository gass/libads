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
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#include "AdsDEF.h"
#include "ads.h"
#include "ads_connect.h"
#include "debugprint.h"


ADSConnection 	**pADSConnectionList = NULL;// filled by ADSsocketGet()
int				nADSConnectionCnt = 0;		// number of currently allocated
											// elements in pADSConnectionList
/**
 * Checks if a connection (socket) to the PLC is already open.
 * If yes, uses the ADSConnection stored in pADSConnectionList,
 * if not, opens a new connection and stores it in pADSConnectionList
 * pADSConnectionList grows dynamically!
 *
 * Returns:	the ADSConnection,
 */
ADSConnection *ADSsocketGet(int dummy, PAmsAddr pAddr, int *adsError)
{
	ADSConnection *dc;
	int i;

	MsgOut(MSG_TRACE, "ADSsocketGet() called\n");
	if(nADSConnectionCnt > 0){
		for(i = 0; i < nADSConnectionCnt; i++){
			if(memcmp((void *)&(pADSConnectionList[i]->partner),
			   (void *)pAddr, sizeof(AmsAddr)) == 0){
				   MsgOut(MSG_SOCKET,
						  MsgStr("ADSsocketGet(): re-using ADSConnection %d\n", i));
				   dc = pADSConnectionList[i];
				   *adsError = 0;
				   MsgOut(MSG_TRACE, "ADSsocketGet() returns a valid ADSConnection\n");
				   return dc;
			   }
		}
	}

	dc = ADSsocketConnect(pAddr, adsError);
	if(!dc){
		MsgOut(MSG_ERROR,
			   "ADSsocketGet(): ADSsocketConnect() returns a NULL ADSConnection.\n");
		return(NULL);
	}

	if(nADSConnectionCnt == 0){
		pADSConnectionList = (ADSConnection **)malloc(sizeof(ADSConnection *));
		nADSConnectionCnt = 1;
		MsgOut(MSG_SOCKET,
			   MsgStr("ADSsocketGet(): initiated ADSConnection to %d entries.\n",
					  nADSConnectionCnt));
	}
	else{
		nADSConnectionCnt++;
		pADSConnectionList = (ADSConnection **)realloc((void *)pADSConnectionList,
		sizeof(ADSConnection *)*nADSConnectionCnt);
		MsgOut(MSG_SOCKET,
			   MsgStr("ADSsocketGet(): growing ADSConnection to %d entries\n",
					  nADSConnectionCnt));
	}
	pADSConnectionList[nADSConnectionCnt-1] = dc;

	*adsError = 0;
	MsgOut(MSG_TRACE, "ADSsocketGet() returns a valid ADSConnection\n");
	return dc;
}

/**
 * \brief Opens a new connection to the PLC identified by pAddr parameter.
 * \param dummy just to have the same parameter list as the router version.
 * \param pAddr Structure with NetId and port number of the ADS server.
 * \param adsError Address of variable that receives the error code.
 * \return An ADSConnection pointer.
 */
ADSConnection *ADSsocketConnect(PAmsAddr pAddr, int *adsError)
{
	struct sockaddr_in 	addr;
	socklen_t 			addrlen;
	int 				socket_fd = 0;

	char 				peer[16];
	int 				opt;
	ADSInterface 		*di;
	ADSConnection 		*dc;
	AmsAddr 			localAmsAddr;
	int					nerr;

	MsgOut(MSG_TRACE, "ADSsocketConnect() called\n");

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	/* Build socket address */
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0xBF02);	/* ADS port 48898 */
	/* lazy convertion from byte array to socket adress format */
	sprintf(peer, "%d.%d.%d.%d", pAddr->netId.b[0], pAddr->netId.b[1],
			pAddr->netId.b[2], pAddr->netId.b[3]);
	inet_aton(peer, &addr.sin_addr);

	/* connect to plc */
	addrlen = sizeof(addr);
	if (connect(socket_fd, (struct sockaddr *) &addr, addrlen)) {
		MsgOut(MSG_ERROR,
			   MsgStr("ADSsocketConnect() socket error connect(): %s.\n",
					  strerror(errno)));
		if(errno == 113) //Linux errno
			*adsError =  0x274c;
		else
			*adsError =  0x1; // for now, should be something meaningfull
		return NULL;
	}
	MsgOut(MSG_SOCKET, MsgStr("ADSsocketConnect() connected to %s\n", peer));

	opt = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, 4)){
		MsgOut(MSG_ERROR,
			   MsgStr("ADSsocketConnect(): setsockopt() fails with "
					   "error code %d: %s.\n", errno, strerror(errno)));
		if(errno == 113) //Linux errno
			*adsError =  0x274c;
		else
			*adsError =  0x1; // for now, should be something meaningfull
		return NULL;
	}

	if((nerr = AdsGetMeAddress(&localAmsAddr, AMSPORT_R0_PLC_RTS1)) != 0x0){
		MsgOut(MSG_ERROR,
			   MsgStr("ADSsocketConnect(): AdsGetMeAddress() fails with "
					   "error code %d\n", nerr));
		*adsError = nerr;
		return(NULL);
	}

	di = _ADSNewInterface(socket_fd, localAmsAddr.netId, pAddr->port, "LinuxADS");
	dc = _ADSNewConnection(di, pAddr->netId, pAddr->port);

	MsgOut(MSG_TRACE, "ADSsocketConnect() returns a vallid ADSConnection\n");
	return(dc);
}

/**
 * Closes a connection to the PLC.
 */
int ADSsocketDisconnect(int *fd)
{
	MsgOut(MSG_TRACE, "ADSsocketDisconnect() called\n");

	if (*fd == 0) {
		MsgOut(MSG_ERROR,
			   "ADSsocketDisconnect() called with file discriptor = 0, "
					   "returns 0xd (error).\n");
		return 0xD;		/* Port not connected */
	}
	close(*fd);
	*fd = 0;

	MsgOut(MSG_TRACE, "ADSsocketDisconnect() returns 0 (OK)\n");
	return 0;
}

/**
 * This is an interface to AdsAPI.c.
 * Used by AdsSyncSetTimeoutEx()
 */
long AdsSetTimeout(long port, long nMs){
	MsgOut(MSG_TRACE, "AdsSetTimeout() called\n");

	if(port <= 0){
		MsgOut(MSG_ERROR,
			   MsgStr("AdsSetTimeout(): returns 0x18, port %d not valid.\n",
					  port));
		return(0x18);
	}
	{
		int i;
		for(i = 0; i < nADSConnectionCnt; i++)
			pADSConnectionList[i]->iface->timeout = nMs;
	}

	MsgOut(MSG_TRACE, "AdsSetTimeout() returns\n");
	return 0x0;
}
