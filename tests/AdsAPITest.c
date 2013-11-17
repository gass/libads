/*
 Implementation of BECKHOFF's ADS protocol.
 ADS = Automation Device Specification
 Implemented according to specifications given in TwinCAT Information System
 May 2011.
 TwinCAT, ADS and maybe other terms used herein are registered trademarks of
 BECKHOFF Company. www.beckhoff.de

 Copyright (C) Luis Matos (gass@otiliamatos.ath.cx) 2009.

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

#include <AdsAPI.h>
#include <AdsDEF.h>

int main(int argc, char **argv)
{
	long nErr;
	AmsAddr addr;
	PAmsAddr pAddr = &addr;

	AdsPortOpen();
	nErr = AdsGetLocalAddress(pAddr);

	if (nErr)
		printf("Error: AdsGetLocalAddress %ld\n", nErr);

	return 0;
}
