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

#ifndef __ADS_CONNECT_H__
#define __ADS_CONNECT_H__

#define ROUTER_PORT 48898 					/* same as the Beckhoff port */
#define CLIENT_PORT AMSPORT_R0_PLC_RTS1

ADSConnection *ADSsocketGet(int dummy, PAmsAddr pAddr, int *adsError);
ADSConnection *ADSsocketConnect(PAmsAddr pAddr, int *adsError);
int ADSsocketDisconnect(ADSConnection *dc);

int	ADScloseConection(int port);
long AdsSetTimeout(long port, long nMs);

#endif //__ADS_CONNECT_H__
