#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.54) 
AC_INIT()
AC_CONFIG_SRCDIR([GUI/webrec.cpp])
AC_CONFIG_HEADER([config.h])

VERSION="0.0.2"

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

gw_CHECK_QT
	
# Checks for libraries.
# FIXME: Replace `main' with a function in `-lX11':
#AC_CHECK_LIB([X11], [main])
# FIXME: Replace `main' with a function in `-lXt':
#AC_CHECK_LIB([Xt], [main])
# FIXME: Replace `main' with a function in `-lXv':
# FIXME: Replace `main' with a function in `-ldl':
#AC_CHECK_LIB([dl], [main])
# FIXME: Replace `main' with a function in `-ljpeg':
#AC_CHECK_LIB([jpeg], [main])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [sin])
AC_CHECK_LIB([Xv], [XvQueryAdaptors])

# Checks for header files.
#AC_PATH_XTRA
# AC_CHECK_LIB(X11, main, echo HAVE_LIBX11, echo LIBX11 missing, $X_LIBS -lX11)
#AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_HEADER_MAJOR
AC_FUNC_MALLOC
AC_HEADER_STDC
AC_FUNC_MMAP
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([clock_gettime gethostbyname gettimeofday inet_ntoa memset munmap select socket strerror])

PKG_CHECK_MODULES(JACK, jack, HAVE_JACK=true, HAVE_JACK=false)
PKG_CHECK_MODULES(THEORA, theora, HAVE_THEORA=true, HAVE_THEORA=false)
PKG_CHECK_MODULES(VORBIS, vorbis, HAVE_VORBIS=true, HAVE_VORBIS=false)
PKG_CHECK_MODULES(VORBISENC, vorbisenc, HAVE_VORBISENC=true, HAVE_VORBISENC=false)

ADD_LIBS="$THEORA_LIBS $VORBIS_LIBS $VORBISENC_LIBS $JACK_LIBS"
ADD_CFLAGS="$THEORA_CFLAGS $VORBIS_CFLAGS $VORBISENC_CFLAGS $JACK_CFLAGS"

AC_SUBST(ADD_LIBS)
AC_SUBST(ADD_CFLAGS)
AC_SUBST(X_CFLAGS)
AC_SUBST(QT_LIBS)
AC_SUBST(VERSION)

AC_CONFIG_FILES([Makefile
		GUI/Makefile])
AC_OUTPUT
