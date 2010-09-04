plugindir = $(VINAGRE_PLUGINS_LIBS_DIR)

plugin_in_files = im-status.vinagre-plugin.in

im-status.vinagre-plugin: im-status.vinagre-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA =  $(plugin_in_files:.vinagre-plugin.in=.vinagre-plugin)

plugin_JAVASCRIPT = im-status.js

EXTRA_DIST = $(plugin_in_files) $(plugin_JAVASCRIPT)
CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

-include $(top_srcdir)/git.mk
