plugindir = $(VINAGRE_PLUGINS_LIBS_DIR)

plugin_in_files = im-status.plugin.desktop.in
%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache
plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

plugin_JAVASCRIPT = im-status.js

EXTRA_DIST = $(plugin_in_files) $(plugin_JAVASCRIPT)
CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

-include $(top_srcdir)/git.mk
