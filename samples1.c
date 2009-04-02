#include <stdio.h>

#ifdef LINUX
#include <unistd.h>
#endif

#include "ADS.h"

AMSNetID me   ={172,16,17,5,1,1};
AMSNetID other={172,16,17,1,1,1};

int main(int argc, char **argv) {
    long nErr, nPort;
    AmsAddr addr;
    PAmsAddr pAddr = &addr;
    DWORD dwData;
    
    nPort = AdsPortOpen();
    nErr = AdsGetLocalAddress(pAddr);
    
    if (nErr) printf("Error: AdsGetLocalAddress %ld\n", nErr);    
    
    pAddr->port= AMSPORT_RO_PLC_RTS0;
    return 0;
}

/*
    Changes:
    
    05/12/2002 created
    23/12/2002 added sys/time.h
*/
