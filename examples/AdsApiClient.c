/*
 Implementation of BECKHOFF's ADS protocol.
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System
 May 2011.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of
 BECKHOFF Company. www.beckhoff.de

 Copyright (C) Luis Matos (gass@otiliamatos.ath.cx) 2009, 2013.

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
#include "AdsDEF.h"
#include "AdsAPI.h"

int main(int argc, char **argv)
{
	short unsigned int nAdsState;
	short unsigned int nDeviceState;
	AdsVersion Version;
	AdsVersion *pVersion = &Version;
	char pDevName[16];
	long nErr;
	AmsAddr Addr;
	PAmsAddr pAddr = &Addr;

	/* Open communication port on the ADS router */
	AdsPortOpen();
	/* set the target ip AMS Net Id      */
	/* When the address is defined */
	//pAddr->netId = (AmsNetId) { {
	//192, 168, 1, 72, 1, 1}};
	/* if the server runs on the local address */
	AdsGetLocalAddress(pAddr);

	/* PLC Port */
	pAddr->port = AMSPORT_LOGGER;

	/* example, read state */
	nErr = AdsSyncReadStateReq(pAddr, &nAdsState, &nDeviceState);
	if (nErr)
		printf("Error: AdsSyncReadStateReq: %ld\n", nErr);
	else {
		printf("AdsState: %d\n", nAdsState);
		printf("DeviceState: %d\n", nDeviceState);
	}

	/* Read device information */
	nErr = AdsSyncReadDeviceInfoReq(pAddr, pDevName, pVersion);
	if (nErr)
		printf("Error: AdsSyncReadDeviceInfoReq: %ld\n", nErr);
	else {
		printf("Name: %s\n", pDevName);
		printf("Version: %d\n", (int)pVersion->version);
		printf("Revision: %d\n", (int)pVersion->revision);
		printf("Build: %d\n", pVersion->build);
	}

	/* Close communication port */
	nErr = AdsPortClose();
	if (nErr)
		printf("Error: AdsPortClose: %ld", nErr);

	return 0;
}
