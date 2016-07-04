AUTOMAKE_OPTIONS = subdir-objects

include Makefile.sources
include $(top_srcdir)/src/gallium/Automake.inc

AM_CFLAGS = \
	$(GALLIUM_DRIVER_CFLAGS) \
	$(LIBUDEV_CFLAGS) \
	$(TEGRA_CFLAGS)

noinst_LTLIBRARIES = libtegra.la

libtegra_la_SOURCES = \
	$(C_SOURCES)

libtegra_la_LIBADD = \
	$(LIBUDEV_LIBS)
