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

/**
  * Establishes a connection (communication port) to the TwinCAT message router.
  * \return A port number that has been assigned to the program by the ADS router is returned. 
  */
int AdsPortOpen(void) {
    if (socket_fd = 0) {
		socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    }
    /*
	 * TODO: Should return a Port number. But ...
	 */
   return 0;
}


/**
 * The connection (communication port) to the TwinCAT message router is closed. 
 * \return The function's error status. 
 */
long AdsPortClose(void) {
	if (socket_fd = 0) {
		return 0xD; /* Port not connected */
	}
	close(socket_fd);
	socket_fd = 0;
	return 0;
}

long AdsGetLocalAddress( PAmsAddr Paddr )  {
	struct ifaddrs *list;
	char done=0;
	unsigned long int temp;
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
			printf ("nome %s\n", cur->ifa_name);
			temp = ntohl (addrStruct->sin_addr.s_addr);
			printf("done 1\n");
			memcpy ((char *) &Paddr->netId, (char *)&temp,4);
			printf ("ip %ld.%ld.%ld.%ld  %d \n", Paddr->netId.b1, Paddr->netId.b2, Paddr->netId.b3, Paddr->netId.b4, cur->ifa_addr->sa_family);
			done = 1;
			break;
		}
	}	
	
	freeifaddrs(list);
	
	if (done) {
		Paddr->netId.b5 = 1;
		Paddr->netId.b6 = 1;		
	}
	else {
		Paddr->netId = (AmsNetId) {172,16,17,3,1,1};
	}
	
	return 0;
} 


