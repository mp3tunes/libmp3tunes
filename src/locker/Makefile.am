noinst_LTLIBRARIES = liblocker.la

liblocker_la_SOURCES = locker.c

liblocker_la_CFLAGS = -I$(top_srcdir)/include $(CURL_CFLAGS) $(LIBXML_CFLAGS) $(LIBCRYPTO_CFLAGS) $(GLIB_CFLAGS)
liblocker_la_LIBADD = $(CURL_LIBS) $(LIBXML_LIBS) $(LIBCRYPTO_LIBS) $(GLIB_LIBS)
