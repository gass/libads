
bin_PROGRAMS = ADSserver AdsApiClient
ADSserver_SOURCES = ADSserver.c \
                    accepter.c \
					ads.h \
					AdsDEF.h \
					AdsAPI.h

ADSserver_CFLAGS = -I$(top_builddir)/src -pthread

ADSserver_LDADD = \
	$(top_builddir)/src/libads.la\
	$(top_builddir)/src/libadsAPI.la
	
AdsApiClient_SOURCES = AdsApiClient.c \
					AdsDEF.h \
					AdsAPI.h

AdsApiClient_CFLAGS = -I$(top_builddir)/src

AdsApiClient_LDADD = \
	$(top_builddir)/src/libads.la\
	$(top_builddir)/src/libadsAPI.la
 
