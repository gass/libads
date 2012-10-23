AC_INIT([libads], [0.1])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_LIBTOOL
AC_PROG_CC

CFLAGS="${CFLAGS} -Wall" 

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	tests/Makefile
	src/Makefile
	src/libads.pc
	examples/Makefile
])
AC_OUTPUT
