#include <stdio.h>

#include <AdsAPI.h>
#include <AdsDEF.h>


int
main (int argc, char **argv)
{
  long nErr;
  AmsAddr addr;
  PAmsAddr pAddr = &addr;

  AdsPortOpen ();
  nErr = AdsGetLocalAddress (pAddr);

  if (nErr)
    printf ("Error: AdsGetLocalAddress %ld\n", nErr);


  return 0;
}
