# reverse-vnc plugin
plugindir = $(VINAGRE_PLUGINS_LIBS_DIR)

INCLUDES = \
	-DDATADIR=\"$(VINAGRE_PLUGINS_DATA_DIR)/reverse-vnc\"	\
	-I$(top_srcdir) 					\
	$(VINAGRE_CFLAGS)					\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED_CFLAGS)

plugin_LTLIBRARIES = libreversevnc.la

libreversevnc_la_SOURCES = 						\
	vinagre-reverse-vnc-plugin.h vinagre-reverse-vnc-plugin.c	\
	vinagre-reverse-vnc-listener.h vinagre-reverse-vnc-listener.c	\
	vinagre-reverse-vnc-listener-dialog.h vinagre-reverse-vnc-listener-dialog.c

if SELF_IFADDRS
libreversevnc_la_SOURCES += if/ifaddrs.h if/getifaddrs.c
endif

libreversevnc_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libreversevnc_la_LIBADD  = $(VNC_LIBS)

uidir = $(VINAGRE_PLUGINS_DATA_DIR)/reverse-vnc
ui_DATA = reverse-vnc.ui

plugin_in_files = reverse-vnc.plugin.desktop.in
%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache
plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

EXTRA_DIST = $(ui_DATA) $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)


-include $(top_srcdir)/git.mk
