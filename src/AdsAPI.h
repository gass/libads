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

#include "AdsDEF.h"
#include <stdint.h>

int32_t AdsGetDllVersion(void);
int32_t AdsPortOpen(void);
int32_t AdsPortClose(void);
int32_t AdsGetLocalAddress(PAmsAddr pAddr);
int32_t AdsSyncWriteControlReq(PAmsAddr pAddr,
                            uint16_t nAdsState,
                            uint16_t nDeviceState,
                            uint32_t nLength,
							void *pData);
int32_t AdsSyncWriteReq(PAmsAddr pAddr,
                            uint32_t nIndexGroup,
                            uint32_t nIndexOffset,
                            uint32_t nLength,
							void *pData);
int32_t AdsSyncReadReq(PAmsAddr pAddr,
                            uint32_t nIndexGroup,
                            uint32_t nIndexOffset,
                            uint32_t nLength,
							void *pData);
int32_t AdsSyncReadReqEx(PAmsAddr pAddr,
                            uint32_t nIndexGroup,
                            uint32_t nIndexOffset,
                            uint32_t nLength,
							void *pData,
                            uint32_t *pcbReturn);
int32_t AdsSyncReadStateReq(PAmsAddr pAddr,
                            uint16_t *pAdsState,
                            uint16_t *pDeviceState);
int32_t AdsSyncReadDeviceInfoReq(PAmsAddr pAddr,
							char *pDevName,
							PAdsVersion pVersion);
int32_t AdsSyncReadWriteReq(PAmsAddr pAddr,
                            uint32_t nIndexGroup,
                            uint32_t nIndexOffset,
                            uint32_t nReadLength,
							void *pReadData,
                            uint32_t nWriteLength,
							void *pWriteData);
int32_t AdsSyncReadWriteReqEx(PAmsAddr pAddr,
                            uint32_t nIndexGroup,
                            uint32_t nIndexOffset,
                            uint32_t nReadLength,
							void *pReadData,
                            uint32_t nWriteLength,
							void *pWriteData,
                            uint32_t *pcbReturn);

int32_t AdsSyncAddDeviceNotificationReq(PAmsAddr pAddr,
                            uint32_t nIndexGroup,
                            uint32_t nIndexOffset,
							PAdsNotificationAttrib pNoteAttrib,
							PAdsNotificationFunc pNoteFunc,
                            uint32_t hUser,
                            uint32_t *pNotification);
int32_t AdsSyncDelDeviceNotificationReq(PAmsAddr pAddr,
                            uint32_t hNotification);
int32_t AdsSyncSetTimeout(int32_t nMs);

//extended functions
int32_t AdsPortOpenEx(void);
int32_t AdsPortCloseEx(long port);
int32_t AdsGetLocalAddressEx(int32_t port, AmsAddr *pAddr);
int32_t AdsSyncWriteReqEx(int32_t	port, 					// Ams port of ADS client
					 	PAmsAddr pAddr,
                        uint32_t nIndexGroup,
                        uint32_t nIndexOffset,
                        uint32_t nLength,
					 	void *pData);
int32_t AdsSyncReadReqEx2(int32_t	port,					// Ams port of ADS client
						PAmsAddr pAddr,
                        uint32_t nIndexGroup,
                        uint32_t nIndexOffset,
                        uint32_t nLength,
						void *pData,
                        uint32_t *pcbReturn);	// count of bytes read

int32_t AdsSyncReadWriteReqEx2(int32_t port,				// Ams port of ADS client
						PAmsAddr pAddr,
                        uint32_t nIndexGroup,
                        uint32_t nIndexOffset,
                        uint32_t nReadLength,
						void *pReadData,
                        uint32_t nWriteLength,
						void *pWriteData,
                        uint32_t *pcbReturn);	// count of bytes read

int32_t AdsSyncReadDeviceInfoReqEx(int32_t port,			// Ams port of ADS client
						PAmsAddr pAddr,
						char *pDevName,
						PAdsVersion pVersion);

int32_t AdsSyncWriteControlReqEx(int32_t port,			// Ams port of ADS client
						PAmsAddr pAddr,
                        uint16_t nAdsState,
                        uint16_t nDeviceState,
                        uint32_t nLength,
						void *pData);

int32_t AdsSyncReadStateReqEx(int32_t port,				// Ams port of ADS client
						PAmsAddr pAddr,
                        uint16_t *pAdsState,
                        uint16_t *pDeviceState);

int32_t AdsSyncAddDeviceNotificationReqEx(int32_t port,	// Ams port of ADS client
						PAmsAddr pAddr,
                        uint32_t nIndexGroup,
                        uint32_t nIndexOffset,
						PAdsNotificationAttrib pNoteAttrib,
						PAdsNotificationFunc pNoteFunc,
                        uint32_t hUser,
                        uint32_t *pNotification);

int32_t AdsSyncDelDeviceNotificationReqEx(int32_t port,	// Ams port of ADS client
						PAmsAddr pAddr,
                        uint32_t hNotification);

int32_t AdsSyncSetTimeoutEx(int32_t port, int32_t nMs);
int32_t AdsSyncGetTimeoutEx(int32_t port, int32_t *pnMs);
int32_t AdsAmsPortEnabledEx(int32_t nPort, char *pbEnabled);


#endif	/* __ADSAPI_H__ */

#ifdef __cplusplus
}
#endif
