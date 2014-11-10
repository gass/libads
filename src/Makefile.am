
lib_LTLIBRARIES = libads.la \
	libadsAPI.la
libads_la_SOURCES = ads.c \
					ads.h \
					AdsDEF.h\
					log2.h\
					ads_io.c\
					ads_io.h\
					ads_connect.c\
					ads_connect.h\
					debugprint.c\
					debugprint.h

libadsAPI_la_SOURCES = \
	AdsAPI.c      \
	AdsAPI.h      \
	AdsDEF.h

libadsAPI_la_LIBADD = \
	$(top_builddir)/src/libads.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA=libads.pc.in

EXTRA_DIST=libads.pc.in 
 
