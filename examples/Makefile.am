
bin_PROGRAMS = ADSserver AdsApiClient ADSclient
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

ADSclient_SOURCES = ADSclient.c \
					AdsDEF.h \
					AdsAPI.h

ADSclient_CFLAGS = -I$(top_builddir)/src

ADSclient_LDADD = \
	$(top_builddir)/src/libads.la
 
