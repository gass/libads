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


#-static -Wl,static -lc.a -static -lpthread.a -nostdlib 
#CFLAGS=-O0 -Wall -Winline

LIBRARIES=libads.so 

all: $(LIBRARIES)
install: libads.so
	cp libads.so /usr/lib
	cp ads.h /usr/include
	ldconfig

ads.o: ads.h log2.h

libads.so: ads.o openSocket.o
	$(LD) -shared ads.o openSocket.o -o libads.so	

clean: 
	rm -f $(DYNAMIC_PROGRAMS)
	rm -f $(PROGRAMS)
	rm -f *.o
	rm -f *.so

