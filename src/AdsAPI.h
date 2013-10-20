/*
    libads is an implementation of the Beckhoff's ADS protocol.

    (C) Luis Matos (gass@otiliamatos.ath.cx) 2009, 2012.

    libads is free software: you can redistribute it and/or modify
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

#ifndef __ADSAPI_H__
#define __ADSAPI_H__

#define ADSAPIERR_NOERROR			0x0000
#include "AdsDEF.h"
int AdsPortOpen(void);

long AdsPortClose(void);

long AdsGetLocalAddress(PAmsAddr pAddr);

long AdsSyncWriteControlReq(PAmsAddr pAddr,
			    unsigned short nAdsState,
			    unsigned short nDeviceState,
			    unsigned long nLength, void *pData);

long AdsSyncWriteReq(PAmsAddr pAddr,
		     unsigned long nIndexGroup,
		     unsigned long nIndexOffset,
		     unsigned long nLength, void *pData);

long AdsSyncReadReq(PAmsAddr pAddr,
		    unsigned long nIndexGroup,
		    unsigned long nIndexOffset,
		    unsigned long nLength, void *pData);

long AdsSyncReadStateReq(PAmsAddr pAddr,
			 unsigned short *pAdsState,
			 unsigned short *pDeviceState);

long AdsSyncReadDeviceInfoReq(PAmsAddr pAddr,
			      char *pDevName, PAdsVersion pVersion);

long AdsSyncReadWriteReq(PAmsAddr pAddr,
			 unsigned long nIndexGroup,
			 unsigned long nIndexOffset,
			 unsigned long nReadLength,
			 void *pReadData,
			 unsigned long nWriteLength, void *pWriteData);

long AdsSyncAddDeviceNotificationReq(PAmsAddr pAddr,
				     unsigned long nIndexGroup,
				     unsigned long nIndexOffset,
				     PAdsNotificationAttrib pNoteAttrib,
				     PAdsNotificationFunc pNoteFunc,
				     unsigned long hUser,
				     unsigned long *pNotification);

#endif				/* __ADSAPI_H__ */
