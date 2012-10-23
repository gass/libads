
#include "AdsAPI.h"
#include "AdsDEF.h"


void main()
{
    ADSSTATE  nAdsState;
    int    nDeviceState;
    long	nErr, nPort;
    AmsAddr   Addr;
    PAmsAddr  pAddr = &Addr;

    // Open communication port on the ADS router
    nPort = AdsPortOpen();
    nErr = AdsGetLocalAddress(pAddr);
    if (nErr) printf("Error: AdsGetLocalAddress %ld\n", nErr);
 
    // TwinCAT2 PLC1 = 801, TwinCAT3 PLC1 = 851
    pAddr->port = 801;

    nErr = AdsSyncReadStateReq(pAddr, &nAdsState, &nDeviceState);
    nErr = AdsSyncReadStateReq(pAddr, &nAdsState, &nDeviceState);
    if (nErr)
        printf("Error: AdsSyncReadStateReq: %ld\n", nErr);
    else
    {
        printf("AdsState: %d\n", nAdsState);
        printf("DeviceState: %d\n", nDeviceState);
    }

    // Close communication port
    nErr = AdsPortClose();
    if (nErr) printf("Error: AdsPortClose: ", nErr);
}

