
AC_PREREQ(2.57)
AC_INIT(xrootd-cmstfc,1.4.3,xrootd-demonstrator@opensciencegrid.org)
AC_CONFIG_SRCDIR([src/XrdCmsTfc.hh])

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

AC_LANG_CPLUSPLUS

AM_INIT_AUTOMAKE

AC_ARG_WITH(xrootd-libdir,
            [  --with-xrootd-libdir=PATH        path to xrootd libraries],
            XROOTD_LIBDIR=-L$withval)
AC_SUBST(XROOTD_LIBDIR)
AC_ARG_WITH(xrootd-incdir,
            [  --with-xrootd-incdir=PATH        path to xrootd header files],
            XROOTD_INCDIR=-I$withval)
AC_SUBST(XROOTD_INCDIR)

# determines shared library needed for loading modules
AC_LIBTOOL_DLOPEN
# init for libtool
AC_PROG_LIBTOOL

AM_PROG_CC_C_O

## libhdfs.so
LDFLAGS="$XROOTD_LIBDIR $LDFLAGS"
AC_CHECK_LIB([xerces-c],[_init],[:],
             AC_MSG_ERROR([Failed linking test for libxerces-c.so])
            )
AC_CHECK_LIB([pcre],[pcre_info], [:],
             AC_MSG_ERROR([Failed linking test for libpcre.so])
            )
LDFLAGS=$LDFLAGS_BUP

CXXFLAGS="$XROOTD_INCDIR $INCLUDES"
#AC_CHECK_HEADER([hdfs.h])
#AC_CHECK_HEADER([jni.h])

CXXFLAGS="$CXXFLAGS -D_REENTRANT -D_FILE_OFFSET_BITS=64"

AC_OUTPUT(Makefile)

