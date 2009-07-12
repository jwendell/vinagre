# vnc plugin
plugindir = $(VINAGRE_PLUGINS_LIBS_DIR)

INCLUDES = \
	-I$(top_srcdir) 				\
	$(VNC_CFLAGS) 					\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)	

plugin_LTLIBRARIES = libvnc.la

libvnc_la_SOURCES = 						\
	vinagre-vnc-plugin.h vinagre-vnc-plugin.c		\
	vinagre-vnc-connection.h vinagre-vnc-connection.c	\
	vinagre-vnc-tab.h vinagre-vnc-tab.c

libvnc_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libvnc_la_LIBADD  = $(VNC_LIBS)

#uidir = $(VINAGRE_PLUGINS_DATA_DIR)/vnc
#ui_DATA = vnc.ui

plugin_in_files = vnc.vinagre-plugin.desktop.in

vnc.vinagre-plugin: vnc.vinagre-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.vinagre-plugin.desktop.in=.vinagre-plugin)

#EXTRA_DIST = $(ui_DATA) $(plugin_in_files)
EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)


-include $(top_srcdir)/git.mk
