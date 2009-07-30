# ssh plugin
plugindir = $(VINAGRE_PLUGINS_LIBS_DIR)

INCLUDES = \
	-I$(top_srcdir) 				\
	$(SSH_CFLAGS) 					\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)	

plugin_LTLIBRARIES = libssh.la

libssh_la_SOURCES = 						\
	vinagre-ssh-plugin.h vinagre-ssh-plugin.c		\
	vinagre-ssh-connection.h vinagre-ssh-connection.c	\
	vinagre-ssh-tab.h vinagre-ssh-tab.c

libssh_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libssh_la_LIBADD  = $(SSH_LIBS)

plugin_in_files = ssh.vinagre-plugin.desktop.in

ssh.vinagre-plugin: ssh.vinagre-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.vinagre-plugin.desktop.in=.vinagre-plugin)

EXTRA_DIST = $(plugin_in_files)
CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

-include $(top_srcdir)/git.mk
