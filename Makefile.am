SUBDIRS = include src examples

EXTRA_DIST = \
	libmp3tunes.pc.in \
	AUTHORS \
	ChangeLog \
	COPYING \
	INSTALL \
	README \
	NEWS

includedir = include/

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmp3tunes.pc

distclean-local:
	$(RM) $(srcdir)/libmp3tunes-*.tar.gz
	$(RM) -r $(srcdir)/autom4te.cache
