# spice plugin
plugindir = $(VINAGRE_PLUGINS_LIBS_DIR)

NULL =
INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_builddir) \
	$(DISABLE_DEPRECATED_CFLAGS)		\
	$(SPICE_CFLAGS)				\
	$(VINAGRE_CFLAGS)			\
	$(WARN_CFLAGS)				\
	$(NULL)

plugin_LTLIBRARIES = libspice.la

libspice_la_SOURCES =						\
	vinagre-spice-plugin.h vinagre-spice-plugin.c		\
	vinagre-spice-connection.h vinagre-spice-connection.c	\
	vinagre-spice-tab.h vinagre-spice-tab.c	\
	vinagre-spice-tunnel.h vinagre-spice-tunnel.c	\
	$(NULL)

libspice_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libspice_la_LIBADD  = $(SPICE_LIBS)

plugin_in_files = spice.plugin.desktop.in
%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache
plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

EXTRA_DIST = $(plugin_in_files)
CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

-include $(top_srcdir)/git.mk
