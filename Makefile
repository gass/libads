#
# These were the paths and names of the ARM cross compiling tools I tried:
# The -Bstatic option was necessary to get statically linked files needed for ARM-simulator
#
# CC=/usr/local/arm/2.95.3/bin/arm-linux-gcc
# LD=/usr/local/arm/2.95.3/bin/arm-linux-ld -Bstatic
# LDFLAGS= -Wl,-Bstatic
##LD=ld
CFLAGS=-Wall -Winline -DLINUX -Dads_LITTLE_ENDIAN
CTFLAGS=-Wall -Winline -fPID -DLINUX -Dads_LITTLE_ENDIAN
CPPFLAGS=-Wall -Winline -DLINUX -Dads_LITTLE_ENDIAN
#
# The following is needed to enable workarounds for statements that do
# not work on (some?) ARM processors:
#
#CFLAGS+=-DARM_FIX 
SRCDIR=src/

#-static -Wl,static -lc.a -static -lpthread.a -nostdlib 
#CFLAGS=-O0 -Wall -Winline

PROGRAMS=samples1 samples2 ADSclient ADSclient2 ADSclient3 ADSserver

DYNAMIC_PROGRAMS=ADSclientd

LIBRARIES=libads.so 

all: $(PROGRAMS) $(LIBRARIES)
install: libads.so
	cp libads.so /usr/lib
	cp ads.h /usr/include
	ldconfig
dynamic: $(DYNAMIC_PROGRAMS)

ads.o: $(SRCDIR)ads.h $(SRCDIR)log2.h

samples1.o: $(SRCDIR)benchmark.c $(SRCDIR)ads.h

samples1: ads.o openSocket.o samples1.o
	$(CC) $(LDFLAGS) ads.o openSocket.o samples1.o -o samples1

samples2.o: $(SRCDIR)benchmark.c $(SRCDIR)ads.h

samples2: ads.o openSocket.o samples2.o
	$(CC) $(LDFLAGS) ads.o openSocket.o samples2.o -o samples2

ADSclient: ads.o openSocket.o ADSclient.o
	$(CC) $(LDFLAGS) ads.o openSocket.o ADSclient.o -o ADSclient

ADSclient2: ads.o openSocket.o ADSclient2.o
	$(CC) $(LDFLAGS) ads.o openSocket.o ADSclient2.o -o ADSclient2

ADSclient3: ads.o openSocket.o ADSclient3.o
	$(CC) $(LDFLAGS) ads.o openSocket.o ADSclient3.o -o ADSclient3

ADSserver: ads.o openSocket.o ADSserver.o
	$(CC) $(LDFLAGS) -lpthread ads.o openSocket.o ADSserver.o -o ADSserver

libads.so: ads.o openSocket.o
	$(LD) -shared ads.o openSocket.o -o libads.so	

clean: 
	rm -f $(DYNAMIC_PROGRAMS)
	rm -f $(PROGRAMS)
	rm -f *.o
	rm -f *.so

