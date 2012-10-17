#include <stdio.h>

#ifdef __linux__
#include <unistd.h>
#endif

#include <AdsAPI.h>
#include <AdsDEF.h>


int main(int argc, char **argv)
{
    long nErr, nPort;
    AmsAddr addr;
    PAmsAddr pAddr = &addr;
    
    nPort = AdsPortOpen();
    nErr = AdsGetLocalAddress(pAddr);
    
    if (nErr) printf("Error: AdsGetLocalAddress %ld\n", nErr);    
   
    
    return 0;
    }
