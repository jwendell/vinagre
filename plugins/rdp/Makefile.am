# rdp plugin
plugindir = $(VINAGRE_PLUGINS_LIBS_DIR)

INCLUDES = \
	-I$(top_srcdir) 				\
	-I$(top_builddir) \
	$(VINAGRE_CFLAGS) 				\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)	

plugin_LTLIBRARIES = librdp.la

librdp_la_SOURCES = 						\
	vinagre-rdp-plugin.h vinagre-rdp-plugin.c		\
	vinagre-rdp-connection.h vinagre-rdp-connection.c	\
	vinagre-rdp-tab.h vinagre-rdp-tab.c

librdp_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
librdp_la_LIBADD  = $(rdp_LIBS)

plugin_in_files = rdp.plugin.desktop.in
%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache
plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

EXTRA_DIST = $(plugin_in_files)
CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

-include $(top_srcdir)/git.mk
