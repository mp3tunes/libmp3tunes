AC_PREREQ(2.61)
AC_INIT(libmp3tunes, 1.0)
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_PROG_CC_C99
if test "x$ac_cv_prog_cc_c99" = "xno"; then
   AC_MSG_ERROR([no C99 compiler found, $PACKAGE_NAME requires a C99 compiler.])
fi

AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_HEADER_STDC

CFLAGS="$CFLAGS -W"

AC_ARG_WITH([locker],
  AS_HELP_STRING([--without-locker],[Disable locker support]),
  , [with_locker=yes])

AM_CONDITIONAL(BUILD_LOCKER_COMPONENTS, [test x$with_locker = xyes])
AS_IF([test x$with_locker = xyes], [
   AC_MSG_NOTICE([BUILDING WITH LOCKER SUPPORT.])

   PKG_CHECK_MODULES(LIBXML, libxml-2.0)
   AC_SUBST(LIBXML_LIBS)
   AC_SUBST(LIBXML_CFLAGS)

   PKG_CHECK_MODULES(CURL, libcurl)
   AC_SUBST(CURL_LIBS)
   AC_SUBST(CURL_CFLAGS)
   
   PKG_CHECK_MODULES(LIBCRYPTO, libcrypto)
   AC_SUBST(LIBCRYPTO_LIBS)
   AC_SUBST(LIBCRYPTO_CFLAGS)
])

AC_ARG_WITH([harmony],
  AS_HELP_STRING([--without-harmony],[Disable harmony support]),
  , [with_harmony=yes])

AM_CONDITIONAL(BUILD_HARMONY_COMPONENTS, [test x$with_harmony = xyes])
AS_IF([test x$with_harmony = xyes], [
   AC_MSG_NOTICE([BUILDING WITH HARMONY SUPPORT.])
   
   PKG_CHECK_MODULES(GLIB, glib-2.0)
   AC_SUBST(GLIB_LIBS)
   AC_SUBST(GLIB_CFLAGS)

   PKG_CHECK_MODULES(GOBJECT, gobject-2.0)
   AC_SUBST(GOBJECT_LIBS)
   AC_SUBST(GOBJECT_CFLAGS)

   PKG_CHECK_MODULES(LOUDMOUTH, loudmouth-1.0)
   AC_SUBST(LOUDMOUTH_LIBS)
   AC_SUBST(LOUDMOUTH_CFLAGS)
])

#AC_ARG_ENABLE(locker,
# [  --enable-locker Enable locker support ],
# [
#	if (test x$enableval = xyes); then
#	   LOCKER_SUBDIRS="locker"
#	   LOCKER_LIBADD="locker/liblocker.la"
#	   EXAMPLE_LOCKER="example_locker"
#	   AC_SUBST(LOCKER_SUBDIRS)
#	   AC_SUBST(LOCKER_LIBADD)
#	   AC_SUBST(EXAMPLE_LOCKER)
#	fi
#
# ], enable_locker=yes )

#AC_ARG_ENABLE(harmony, 
# [  --enable-harmony Enable harmony support ],
# [
#	if (test x$enableval = xyes); then
#	   HARMONY_SUBDIRS="harmony"
#	   HARMONY_LIBADD="harmony/libharmony.la"
#	   EXAMPLE_DAEMON="example_daemon"
#	   AC_SUBST(HARMONY_SUBDIRS)
#	   AC_SUBST(HARMONY_LIBADD)
#	   AC_SUBST(EXAMPLE_DAEMON)
#	fi
#
#])

AC_CONFIG_FILES([
Makefile
libmp3tunes.pc
include/Makefile
src/locker/Makefile
src/harmony/Makefile
src/Makefile
examples/Makefile
])

AC_OUTPUT
