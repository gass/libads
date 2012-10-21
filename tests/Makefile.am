
bin_PROGRAMS = AdsAPITest adsTest
AdsAPITest_SOURCES = AdsAPITest.c \
					ads.h \
					AdsDEF.h \
					AdsAPI.h
AdsAPITest_CFLAGS = -I$(top_builddir)/src

AdsAPITest_LDADD = \
	$(top_builddir)/src/libads.la\
	$(top_builddir)/src/libadsAPI.la
	
adsTest_SOURCES = adsTest.c \
					ads.h
adsTest_CFLAGS = -I$(top_builddir)/src

adsTest_LDADD = \
	$(top_builddir)/src/libads.la
 
