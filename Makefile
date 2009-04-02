#
# These were the paths and names of the ARM cross compiling tools I tried:
# The -Bstatic option was necessary to get statically linked files needed for ARM-simulator
#
# CC=/usr/local/arm/2.95.3/bin/arm-linux-gcc
# LD=/usr/local/arm/2.95.3/bin/arm-linux-ld -Bstatic
# LDFLAGS= -Wl,-Bstatic
##LD=ld
CFLAGS=-Wall -Winline -DLINUX -DADS_LITTLE_ENDIAN
CTFLAGS=-Wall -Winline -fPID -DLINUX -DADS_LITTLE_ENDIAN
CPPFLAGS=-Wall -Winline -DLINUX -DADS_LITTLE_ENDIAN
#
# The following is needed to enable workarounds for statements that do
# not work on (some?) ARM processors:
#
#CFLAGS+=-DARM_FIX 


#-static -Wl,static -lc.a -static -lpthread.a -nostdlib 
#CFLAGS=-O0 -Wall -Winline

PROGRAMS=samples1 samples2 ADSclient ADSclient2 ADSclient3 ADSserver

DYNAMIC_PROGRAMS=ADSclientd

LIBRARIES=libADS.so 

all: $(PROGRAMS) $(LIBRARIES)
install: libADS.so
	cp libADS.so /usr/lib
	cp ADS.h /usr/include
	ldconfig
dynamic: $(DYNAMIC_PROGRAMS)

ADS.o: ADS.h log2.h

samples1.o: benchmark.c ADS.h

samples1: ADS.o openSocket.o samples1.o
	$(CC) $(LDFLAGS) ADS.o openSocket.o samples1.o -o samples1

samples2.o: benchmark.c ADS.h

samples2: ADS.o openSocket.o samples2.o
	$(CC) $(LDFLAGS) ADS.o openSocket.o samples2.o -o samples2

ADSclient: ADS.o openSocket.o ADSclient.o
	$(CC) $(LDFLAGS) ADS.o openSocket.o ADSclient.o -o ADSclient

ADSclient2: ADS.o openSocket.o ADSclient2.o
	$(CC) $(LDFLAGS) ADS.o openSocket.o ADSclient2.o -o ADSclient2

ADSclient3: ADS.o openSocket.o ADSclient3.o
	$(CC) $(LDFLAGS) ADS.o openSocket.o ADSclient3.o -o ADSclient3

ADSserver: ADS.o openSocket.o ADSserver.o
	$(CC) $(LDFLAGS) -lpthread ADS.o openSocket.o ADSserver.o -o ADSserver




libADS.so: ADS.o openSocket.o
	$(LD) -shared ADS.o openSocket.o -o libADS.so	

ibhsim5.o: simProperties.c
ibhsim5: ibhsim5.o nodave.h nodave.o openSocket.o openSocket.h
	$(CC) -lpthread ibhsim5.o openSocket.o nodave.o -o ibhsim5
isotest4: isotest4.o openSocket.o nodave.o nodave.h
	$(CC) $(LDFLAGS) -lpthread isotest4.o openSocket.o nodave.o $(LIB) -o isotest4

clean: 
	rm -f $(DYNAMIC_PROGRAMS)
	rm -f $(PROGRAMS)
	rm -f *.o
	rm -f *.so

