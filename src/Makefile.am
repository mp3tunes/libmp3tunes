lib_LTLIBRARIES = libmp3tunes.la

SUBDIRS = 

libmp3tunes_la_LIBADD =

if BUILD_LOCKER_COMPONENTS

SUBDIRS += locker
libmp3tunes_la_LIBADD += locker/liblocker.la

endif 

if BUILD_HARMONY_COMPONENTS

SUBDIRS += harmony
libmp3tunes_la_LIBADD += harmony/libharmony.la

endif

libmp3tunes_la_SOURCES = mp3tunes.c

libmp3tunes_la_LDFLAGS = -version-info 1:0:0
libmp3tunes_la_CFLAGS = -I../include 
