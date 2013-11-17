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

#ifndef __ADSAPI_H__
#define __ADSAPI_H__

long AdsGetDllVersion(void);
long AdsPortOpen(void);
long AdsPortClose(void);
long AdsGetLocalAddress(PAmsAddr pAddr);
long AdsSyncWriteControlReq(PAmsAddr pAddr,
			    			unsigned short nAdsState,
			   				unsigned short nDeviceState,
			    			unsigned long nLength,
							void *pData);
long AdsSyncWriteReq(PAmsAddr pAddr,
							unsigned long nIndexGroup,
							unsigned long nIndexOffset,
							unsigned long nLength,
							void *pData);
long AdsSyncReadReq(PAmsAddr pAddr,
							unsigned long nIndexGroup,
							unsigned long nIndexOffset,
							unsigned long nLength,
							void *pData);
long AdsSyncReadReqEx(PAmsAddr pAddr,
							unsigned long nIndexGroup,
							unsigned long nIndexOffset,
							unsigned long nLength,
							void *pData,
							unsigned long *pcbReturn);
long AdsSyncReadStateReq(PAmsAddr pAddr,
							unsigned short *pAdsState,
							unsigned short *pDeviceState);
long AdsSyncReadDeviceInfoReq(PAmsAddr pAddr,
							char *pDevName,
							PAdsVersion pVersion);
long AdsSyncReadWriteReq(PAmsAddr pAddr,
							unsigned long nIndexGroup,
							unsigned long nIndexOffset,
							unsigned long nReadLength,
							void *pReadData,
							unsigned long nWriteLength,
							void *pWriteData);
long AdsSyncReadWriteReqEx(PAmsAddr pAddr,
							unsigned long nIndexGroup,
							unsigned long nIndexOffset,
							unsigned long nReadLength,
							void *pReadData,
							unsigned long nWriteLength,
							void *pWriteData,
							unsigned long *pcbReturn);

long AdsSyncAddDeviceNotificationReq(PAmsAddr pAddr,
							unsigned long nIndexGroup,
							unsigned long nIndexOffset,
							PAdsNotificationAttrib pNoteAttrib,
							PAdsNotificationFunc pNoteFunc,
							unsigned long hUser,
							unsigned long *pNotification);
long AdsSyncDelDeviceNotificationReq(PAmsAddr pAddr,
							unsigned long hNotification);
long AdsSyncSetTimeout(long nMs);

//extended functions
long AdsPortOpenEx(void);
long AdsPortCloseEx(long port);
long AdsGetLocalAddressEx(long port, AmsAddr *pAddr);
long AdsSyncWriteReqEx(long	port, 					// Ams port of ADS client
					 	PAmsAddr pAddr,
					 	unsigned long nIndexGroup,
					 	unsigned long nIndexOffset,
					 	unsigned long nLength,
					 	void *pData);
long AdsSyncReadReqEx2(long	port,					// Ams port of ADS client
						PAmsAddr pAddr,
						unsigned long nIndexGroup,
						unsigned long nIndexOffset,
						unsigned long nLength,
						void *pData,
						unsigned long *pcbReturn);	// count of bytes read

long AdsSyncReadWriteReqEx2(long port,				// Ams port of ADS client
						PAmsAddr pAddr,
						unsigned long nIndexGroup,
						unsigned long nIndexOffset,
						unsigned long nReadLength,
						void *pReadData,
						unsigned long nWriteLength,
						void *pWriteData,
						unsigned long *pcbReturn);	// count of bytes read

long AdsSyncReadDeviceInfoReqEx(long port,			// Ams port of ADS client
						PAmsAddr pAddr,
						char *pDevName,
						PAdsVersion pVersion);

long AdsSyncWriteControlReqEx(long port,			// Ams port of ADS client
						PAmsAddr pAddr,
						unsigned short nAdsState,
						unsigned short nDeviceState,
						unsigned long nLength,
						void *pData);

long AdsSyncReadStateReqEx(long port,				// Ams port of ADS client
						PAmsAddr pAddr,
						unsigned short *pAdsState,
						unsigned short *pDeviceState);

long AdsSyncAddDeviceNotificationReqEx(long port,	// Ams port of ADS client
						PAmsAddr pAddr,
						unsigned long nIndexGroup,
						unsigned long nIndexOffset,
						PAdsNotificationAttrib pNoteAttrib,
						PAdsNotificationFunc pNoteFunc,
						unsigned long hUser,
						unsigned long *pNotification);

long AdsSyncDelDeviceNotificationReqEx(long port,	// Ams port of ADS client
						PAmsAddr pAddr,
						unsigned long hNotification);

long AdsSyncSetTimeoutEx(long port, long nMs);
long AdsSyncGetTimeoutEx(long port, long *pnMs);
long AdsAmsPortEnabledEx(long nPort, char *pbEnabled);


#endif	/* __ADSAPI_H__ */

#ifdef __cplusplus
}
#endif
