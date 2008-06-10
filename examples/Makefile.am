noinst_PROGRAMS =
if BUILD_LOCKER_COMPONENTS

noinst_PROGRAMS += example_locker
example_locker_SOURCES = example_locker.c
example_locker_CFLAGS = -I../include @LIBXML_CFLAGS@ @CURL_CFLAGS@ @LOUDMOUTH_CFLAGS@ @GLIB_CFLAGS@ @GOBJECT_CFLAGS@
example_locker_LDADD = ../src/libmp3tunes.la  @LIBXML_LIBS@ @CURL_LIBS@ @LOUDMOUTH_LIBS@ @GLIB_LIBS@ @GOBJECT_LIBS@

endif

if BUILD_HARMONY_COMPONENTS

noinst_PROGRAMS += example_daemon
example_daemon_SOURCES = example_daemon.c
example_daemon_CFLAGS = -I../include @LIBXML_CFLAGS@ @CURL_CFLAGS@ @LOUDMOUTH_CFLAGS@ @GLIB_CFLAGS@ @GOBJECT_CFLAGS@
example_daemon_LDADD = ../src/libmp3tunes.la @LIBXML_LIBS@ @CURL_LIBS@ @LOUDMOUTH_LIBS@ @GLIB_LIBS@ @GOBJECT_LIBS@

endif






