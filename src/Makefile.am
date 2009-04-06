INCLUDES = \
	$(WARN_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(DEPRECATED_FLAGS) \
	$(LIBADS_CFLAGS) 


lib_LTLIBRARIES = libads.la
libads_la_SOURCES = ads.c

configdir = $(libdir)/pkgconfig
pkgconfig_DATA=libads.pc.in
EXTRA_DIST=libads.pc.in
 
