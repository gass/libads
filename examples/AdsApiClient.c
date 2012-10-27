
#include <stdio.h>
#include "AdsAPI.h"
#include "AdsDEF.h"

int main(int argc, char **argv)
{
	short unsigned int nAdsState;
	short unsigned int nDeviceState;
	long nErr;
	AmsAddr Addr;
	PAmsAddr pAddr = &Addr;

	// Open communication port on the ADS router
	AdsPortOpen();
	nErr = AdsGetLocalAddress(pAddr);
	if (nErr)
		printf("Error: AdsGetLocalAddress %ld\n", nErr);

	// TwinCAT2 PLC1 = 801, TwinCAT3 PLC1 = 851
	pAddr->port = 801;

	nErr = AdsSyncReadStateReq(pAddr, &nAdsState, &nDeviceState);
	nErr = AdsSyncReadStateReq(pAddr, &nAdsState, &nDeviceState);
	if (nErr)
		printf("Error: AdsSyncReadStateReq: %ld\n", nErr);
	else {
		printf("AdsState: %d\n", nAdsState);
		printf("DeviceState: %d\n", nDeviceState);
	}

	// Close communication port
	nErr = AdsPortClose();
	if (nErr)
		printf("Error: AdsPortClose: %ld", nErr);

	return 0;
}
