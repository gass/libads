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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <syslog.h>

#include "AdsDEF.h"
#include "ads.h"
#include "ads_io.h"
#include "debugprint.h"

/**
 * @brief Send a packet to a peer (router or client)
 * @param di	interface to use for reading
 * @param p 	packet to send
 * @param error where to store errno in case of a system error
 * @return   1: success;
 * @return   0: failure (errno is copied to *error param)
 * @return  -3: failure, internal error (ADSInterface = NULL)
 * @return  -4: failure, ADSInterface has error flag set
 */
int	_ADSWritePacket(ADSInterface *di, ADSpacket *p, int *error)
{
	int rc;

	MsgOut(MSG_TRACE, "_ADSWritePacket() called\n");

	if (di == NULL){
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR,
			   "_ADSWritePacket() called with NULL-ADSInterface.");
#endif
		MsgOut(MSG_ERROR,
				"_ADSWritePacket() called with NULL-ADSInterface, "
						"returns: -12.\n");
		if(error)
			*error = 0;
		return -3;
	}
	if (di->error){
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR,
			   "_ADSWritePacket() ADSInterface has error flag set!");
#endif
		MsgOut(MSG_ERROR,
				"_ADSWritePacket() ADSInterface has error flag set!, "
						"returns: -11.\n");
		if(error)
			*error = 0;
		return -4;
	}

	rc = send(di->sd,
			  (void *)p,
			  sizeof(AMS_TCPheader) + p->adsHeader.length,
			  MSG_NOSIGNAL);
	// TODO: return ADS error code instead of linux errno
	if(rc == -1){
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR,
			   "_ADSWritePacket(): send() failed: %s.", strerror(errno));
#endif
		MsgOut(MSG_ERROR,
			   MsgStr("_ADSWritePacket(): send() failed: %s.\n", strerror(errno)));
		if(error)
			*error = errno;
		return(0);
	}
	else if(rc != p->adsHeader.length + sizeof(AMS_TCPheader)){
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR,
			   		"_ADSWritePacket(): Sent %d Bytes, but %d were requested!",
			   			rc, sizeof(AMS_TCPheader) + p->adsHeader.length );
#endif
		MsgOut(MSG_ERROR,
			   MsgStr("_ADSWritePacket(): Sent %d Bytes, but %d were requested!\n",
					  rc, sizeof(AMS_TCPheader) + p->adsHeader.length ));
		if(error)
			*error = errno;
		return(0);
	}

	MsgOut(MSG_TRACE, "_ADSWritePacket() returns 1 (OK)\n");
	return 1;
}

/**
 * @brief Read one byte, may run into timeout
 *
 * @param rfd	file descriptor to use for reading
 * @param b 	where to store retrieved byte
 * @param pt 	pointer to timeval with time left bevore timeout occures
 * @param error see return values
 * @return 		1: OK
 * @return		0: select() or recv() error (errno is in error param)
 * @return	   -1: time out, error param = 0
 * @return	   -2: peer shut down, error param = 0
 */
int _ADSReadByte(int rfd, unsigned char *b, struct timeval *pt, int *error)
{
	fd_set FDS;
	int rc;

	FD_ZERO(&FDS);
	FD_SET(rfd, &FDS);

// 	if(pt)
// 		MsgOut(MSG_DEVEL, MsgStr("timeout remaining %d.%d\n", pt->tv_sec, pt->tv_usec));
	rc = select(rfd + 1, &FDS, NULL, NULL, pt);
	if (rc == -1){
#ifdef LOG_ALL_MESSAGES
		(LOG_USER | LOG_ERR,
			   "_ADSReadByte(): select() failed: %s.", strerror(errno));
#endif
		MsgOut(MSG_ERROR,
			   MsgStr("_ADSReadByte(): select() failed: %s.\n",
					  strerror(errno)));
		//TODO translate linux errno to ADS error !
		if(error)
			*error = errno;
		return (0);
	}
	else if (rc == 0){
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR, "_ADSReadByte(): select() timed out.");
#endif
		MsgOut(MSG_ERROR, "_ADSReadByte(): select() timed out.\n");
		if(error)
			*error = 0;
		return (-1);
	}

	rc = recv(rfd, b, 1, 0);
	if (rc == 0){
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR,
			   "_ADSReadByte(): recv() dedected peer shut down.");
#endif
		MsgOut(MSG_ERROR,
			   "_ADSReadByte(): recv() dedected peer shut down.\n");
		*error = 0;
		return (-2);
	}
	else if (rc == -1){
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR,
			   "_ADSReadByte(): recv() failed: %s.", strerror(errno));
#endif
		MsgOut(MSG_ERROR,
			   MsgStr("_ADSReadByte(): recv() failed: %s.\n",
					  strerror(errno)));
		//TODO translate linux errno to ADS error !
		if(error)
			*error = errno;
		return (0);
	}

	if(error)
		*error = 0;
	return(1);
}

/**
 * @brief Read one complete packet, may run into timeout
 *
 * @param di	interface to use for reading
 * @param b 	where to store retrieved packet
 * @param error where to store errno in case of a system error
 * @return 	 >0: OK, number of bytes read
 * @return 	  0: select() or recv() error (errno is in error param)
 * @return 	 -1: time out, error param = 0
 * @return 	 -2: peer shut down, error param = 0
 * @return   -3: failure, internal error (ADSInterface = NULL)
 * @return   -4: failure, ADSInterface has error flag set
 */
int _ADSReadPacket(ADSInterface *di, unsigned char *b, int *error)
{
	AMS_TCPheader *h;
	struct timeval t, *pt;
	int rc, res = 0;

	MsgOut(MSG_TRACE, "_ADSReadPacket() called\n");

	if (di == NULL){
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR, "_ADSReadPacket() called with NULL-ADSInterface.");
#endif
		MsgOut(MSG_ERROR,
			   "_ADSReadPacket() called with NULL-ADSInterface, "
					   "returns: -12\n");
		if(error)
			*error = 0;
		return -12;
	}
	if (di->error){
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR,
			   "_ADSReadPacket() ADSInterface has error flag set.");
#endif
		MsgOut(MSG_ERROR,
			   "_ADSReadPacket() ADSInterface has error flag set!, "
					   "returns: -11\n");
		if(error)
			*error = 0;
		return -11;
	}

	// We want to read the whole packet within "timeout" ms.
	// select() updates the timeout argument to indicate how much time was left
	// so we pass the timeval struct to each call without reinitializing it!
	if(di->timeout != 0){
		t.tv_sec = di->timeout / 1000; 				//Sec
		t.tv_usec = (di->timeout % 1000L) * 1000000L;	//uSec
		pt = &t;
		MsgOut(MSG_DEVEL,
			   MsgStr("_ADSReadPacket(): timeout set to %d ms\n", di->timeout));
	}
	else
		pt = NULL;

	while (res < sizeof(AMS_TCPheader)) {
			rc = _ADSReadByte(di->sd, b + res, pt, error);
		if(rc == 1)
			res++;
		else{
#ifdef LOG_ALL_MESSAGES
			syslog(LOG_USER | LOG_ERR,
				   "_ADSReadPacket(): _ADSReadByte() returned: %d (ERROR).", rc);
#endif
		MsgOut(MSG_ERROR,
				   MsgStr("_ADSReadPacket(): _ADSReadByte() returned: %d (ERROR).\n", rc));
			return rc;
		}
	}
	h = (AMS_TCPheader *)b;
	MsgOut(MSG_PACKET_V,
		   MsgStr("_ADSReadPacket(): AMS_TCPheader.length= %d\n", h->length));

	while (res < sizeof(AMS_TCPheader) + h->length) {
		rc = _ADSReadByte(di->sd, b + (res < MAXDATALEN ? res : MAXDATALEN - 1), pt, error);
		if(rc == 1){
			res++;
		}
		else{
#ifdef LOG_ALL_MESSAGES
			syslog(LOG_USER | LOG_ERR,
				   "_ADSReadPacket(): _ADSReadByte() returned: %d (ERROR).", rc);
#endif
			MsgOut(MSG_ERROR,
				   MsgStr("_ADSReadPacket(): _ADSReadByte() returned: %d (ERROR).\n",
						  rc));
			return rc;
		}
	}

	MsgOut(MSG_PACKET_V,
		   MsgStr("_ADSReadPacket(): %d bytes read, %d needed\n",
				  res, sizeof(AMS_TCPheader) + h->length));

	if (sizeof(AMS_TCPheader) + h->length > MAXDATALEN) {
#ifdef LOG_ALL_MESSAGES
		syslog(LOG_USER | LOG_ERR,
				   "_ADSReadPacket(): packet to long: %d bytes (max is %d)\n",
						sizeof(AMS_TCPheader) + h->length, MAXDATALEN);
#endif
		MsgOut(MSG_ERROR,
		   	MsgStr("_ADSReadPacket(): packet to long: %d bytes (max is %d)\n",
				sizeof(AMS_TCPheader) + h->length, MAXDATALEN));

		h->length = MAXDATALEN - sizeof(AMS_TCPheader);

		unsigned long *result;
		result = (unsigned long *)(b + sizeof(AMS_TCPheader)+ sizeof(AMSheader));
		*result = 0xe;

		if(error)
			*error = 0xe;
		return (res);
	}

	MsgOut(MSG_TRACE, MsgStr("_ADSReadPacket() returns: %d (OK)\n", res));

	if(error)
		*error = 0;
	return (res);
}
