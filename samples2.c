#include <stdio.h>

#ifdef LINUX
#include <unistd.h>
#endif

#include "ADS.h"

AMSNetID me   ={172,16,17,5,1,1};
AMSNetID other={172,16,17,1,1,1};

int main(int argc, char **argv) {
    ADSparseNetID("172.116.17.5.15.1",&me);
    printf ("%d.%d\n",me.b4,me.b5);
    return 0;
}

/*
    Changes:
    
    05/12/2002 created
    23/12/2002 added sys/time.h
*/
