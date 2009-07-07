# dummy plugin
plugindir = $(VINAGRE_PLUGINS_LIBS_DIR)

INCLUDES = \
	-I$(top_srcdir) 				\
	$(VINAGRE_CFLAGS) 				\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)	

plugin_LTLIBRARIES = libdummy.la

libdummy_la_SOURCES = \
	vinagre-dummy-plugin.h	\
	vinagre-dummy-plugin.c

libdummy_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libdummy_la_LIBADD  = $(VINAGRE_LIBS)

uidir = $(VINAGRE_PLUGINS_DATA_DIR)/dummy
ui_DATA = dummy.ui

plugin_in_files = dummy.vinagre-plugin.desktop.in

dummy.vinagre-plugin: dummy.vinagre-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.vinagre-plugin.desktop.in=.vinagre-plugin)

EXTRA_DIST = $(ui_DATA) $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)


-include $(top_srcdir)/git.mk
