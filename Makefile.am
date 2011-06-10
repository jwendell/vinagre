ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
SUBDIRS = help po

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper

common_cppflags = \
	-I$(top_builddir) \
	-I$(top_builddir)/vinagre \
	-I$(top_srcdir)/vinagre/view \
	-DBINDIR=\"$(bindir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DPREFIX=\""$(prefix)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DLIBDIR=\""$(libdir)"\" \
	-DVINAGRE_DATADIR=\""$(pkgdatadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\" \
	-DSSH_PROGRAM=\"$(SSH_PROGRAM)\" \
	$(VINAGRE_CFLAGS) \
	$(WARN_FLAGS) \
	$(DISABLE_DEPRECATED_FLAGS)

AM_CPPFLAGS = \
	$(common_cppflags) \
	-include config.h

AM_VALAFLAGS = \
	--vapidir vinagre/vapi \
	--pkg config \
	--pkg gtk+-3.0

bin_PROGRAMS = vinagre/vinagre

vinagre_vinagre_SOURCES = \
	vinagre/vinagre-main.c \
	vinagre/vinagre-options.c

vinagre_vinagre_LDADD = \
	libvinagre.la \
	$(VINAGRE_LIBS) \
	$(GTKVNC_LIBS) \
	$(rdp_plugin) \
	$(spice_plugin) \
	$(ssh_plugin) \
	libvnc.la

vinagrehdir = $(includedir)/vinagre-@VINAGRE_ABI_VERSION@/vinagre

noinst_vinagreh_headers = \
	vinagre/view/autoDrawer.h \
	vinagre/view/drawer.h \
	vinagre/view/ovBox.h \
	vinagre/vinagre-window-private.h \
	vinagre/pty_open.h \
	vinagre/vinagre-options.h

vinagreh_HEADERS = \
	vinagre/vinagre-bookmarks-entry.h \
	vinagre/vinagre-bookmarks.h \
	vinagre/vinagre-bookmarks-migration.h \
	vinagre/vinagre-bookmarks-tree.h \
	vinagre/vinagre-bookmarks-ui.h \
	vinagre/vinagre-commands.h \
	vinagre/vinagre-connect.h \
	vinagre/vinagre-connection.h \
	vinagre/vinagre-debug.h \
	vinagre/vinagre-dirs.h \
	vinagre/vinagre-fav.h \
	vinagre/vinagre-notebook.h \
	vinagre/vinagre-prefs.h \
	vinagre/vinagre-static-extension.h \
	vinagre/vinagre-tab.h \
	vinagre/vinagre-ui.h \
	vinagre/vinagre-window.h \
	vinagre/vinagre-dnd.h \
	vinagre/vinagre-ssh.h \
	vinagre/vinagre-cache-prefs.h \
	vinagre/vinagre-protocol.h \
	vinagre/vinagre-plugins-engine.h

# The main library
noinst_LTLIBRARIES = libvinagre.la

handwritten_sources = \
	vinagre/view/autoDrawer.c \
	vinagre/view/drawer.c \
	vinagre/view/ovBox.c \
	vinagre/vinagre-bookmarks.c \
	vinagre/vinagre-bookmarks-entry.c \
	vinagre/vinagre-bookmarks-migration.c \
	vinagre/vinagre-bookmarks-tree.c \
	vinagre/vinagre-bookmarks-ui.c \
	vinagre/vinagre-commands.c \
	vinagre/vinagre-connect.c \
	vinagre/vinagre-connection.c \
	vinagre/vinagre-debug.c \
	vinagre/vinagre-dirs.c \
	vinagre/vinagre-fav.c \
	vinagre/vinagre-notebook.c \
	vinagre/vinagre-prefs.c \
	vinagre/vinagre-static-extension.c \
	vinagre/vinagre-tab.c \
	vinagre/vinagre-utils.vala \
	vinagre/vinagre-window.c \
	vinagre/pty_open.c \
	vinagre/vinagre-ssh.c \
	vinagre/vinagre-cache-prefs.c \
	vinagre/vinagre-protocol.c \
	vinagre/vinagre-plugins-engine.c

libvinagre_la_SOURCES = \
	$(handwritten_sources)

if VINAGRE_HAVE_AVAHI
libvinagre_la_SOURCES += \
	vinagre/vinagre-mdns.c
noinst_vinagreh_headers += \
	vinagre/vinagre-mdns.h
endif

libvinagre_la_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"

libvinagre_la_LIBADD = $(VINAGRE_LIBS)

# Telepathy stuff
if VINAGRE_HAVE_TELEPATHY_GLIB
handwritten_sources += \
	vinagre/vinagre-tubes-manager.c \
	vinagre/vinagre-tube-handler.c
noinst_vinagreh_headers += \
	vinagre/vinagre-tubes-manager.h \
	vinagre/vinagre-tube-handler.h
endif

plugindir = $(VINAGRE_PLUGINS_LIBS_DIR)

noinst_plugin_headers = \
	plugins/rdp/vinagre-rdp-plugin.h \
	plugins/rdp/vinagre-rdp-connection.h \
	plugins/rdp/vinagre-rdp-tab.h \
	plugins/spice/vinagre-spice-plugin.h \
	plugins/spice/vinagre-spice-connection.h \
	plugins/spice/vinagre-spice-tab.h \
	plugins/spice/vinagre-spice-tunnel.h \
	plugins/ssh/vinagre-ssh-plugin.h \
	plugins/ssh/vinagre-ssh-connection.h \
	plugins/ssh/vinagre-ssh-tab.h \
	plugins/reverse-vnc/if/ifaddrs.h \
	plugins/reverse-vnc/vinagre-reverse-vnc-plugin.h \
	plugins/reverse-vnc/vinagre-reverse-vnc-listener.h \
	plugins/reverse-vnc/vinagre-reverse-vnc-listener-dialog.h \
	plugins/vnc/vinagre-vnc-plugin.h \
	plugins/vnc/vinagre-vnc-connection.h \
	plugins/vnc/vinagre-vnc-tab.h \
	plugins/vnc/vinagre-vnc-tunnel.h

if VINAGRE_ENABLE_RDP
rdp_plugin = librdp.la
rdp_plugin_in = plugins/rdp/rdp.plugin.desktop.in

librdp_la_SOURCES = \
	plugins/rdp/vinagre-rdp-plugin.c \
	plugins/rdp/vinagre-rdp-connection.c \
	plugins/rdp/vinagre-rdp-tab.c

librdp_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
librdp_la_LIBADD = $(VINAGRE_LIBS)
endif

if VINAGRE_ENABLE_SPICE
spice_plugin = libspice.la
spice_plugin_in = plugins/spice/spice.plugin.desktop.in

libspice_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(SPICE_CFLAGS) \
	-I$(top_srcdir)/plugins/spice

libspice_la_SOURCES = \
	plugins/spice/vinagre-spice-plugin.c \
	plugins/spice/vinagre-spice-connection.c \
	plugins/spice/vinagre-spice-tab.c \
	plugins/spice/vinagre-spice-tunnel.c

libspice_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libspice_la_LIBADD = $(SPICE_LIBS)
endif

if VINAGRE_ENABLE_SSH
ssh_plugin = libssh.la
ssh_plugin_in = plugins/ssh/ssh.plugin.desktop.in

libssh_la_SOURCES = \
	plugins/ssh/vinagre-ssh-plugin.c \
	plugins/ssh/vinagre-ssh-connection.c \
	plugins/ssh/vinagre-ssh-tab.c

libssh_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libssh_la_LIBADD = $(VINAGRE_LIBS)
endif

plugin_LTLIBRARIES = \
	$(rdp_plugin) \
	$(spice_plugin) \
	$(ssh_plugin) \
	libreversevnc.la \
	libvnc.la

if VINAGRE_HAVE_SELF_IFADDRS
ifaddrs_sources = \
	plugins/reverse-vnc/if/getifaddrs.c
endif

libreversevnc_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir) \
	-DVINAGRE_REVERSE_VNC_DATADIR=\"$(reversevncuidir)\"

libreversevnc_la_SOURCES = \
	plugins/reverse-vnc/vinagre-reverse-vnc-plugin.c \
	plugins/reverse-vnc/vinagre-reverse-vnc-listener.c \
	plugins/reverse-vnc/vinagre-reverse-vnc-listener-dialog.c \
	$(ifaddrs_sources)

libreversevnc_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libreversevnc_la_LIBADD = $(VNC_LIBS)

reversevncuidir = $(VINAGRE_PLUGINS_DATA_DIR)/reverse-vnc
dist_reversevncui_DATA = plugins/reverse-vnc/reverse-vnc.ui

libvnc_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(VNC_CFLAGS) \
	-I$(top_srcdir)

libvnc_la_SOURCES = \
	plugins/vnc/vinagre-vnc-plugin.c \
	plugins/vnc/vinagre-vnc-connection.c \
	plugins/vnc/vinagre-vnc-tab.c \
	plugins/vnc/vinagre-vnc-tunnel.c

libvnc_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libvnc_la_LIBADD = $(VNC_LIBS)

plugin_in_files = \
	$(rdp_plugin_in) \
	$(spice_plugin_in) \
	$(ssh_plugin_in) \
	plugins/reverse-vnc/reverse-vnc.plugin.desktop.in \
	plugins/vnc/vnc.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po)
	$(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

nodist_plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

iconthemedir = $(datadir)/icons/hicolor
mimeicon16dir = $(iconthemedir)/16x16/mimetypes
mimeicon22dir = $(iconthemedir)/22x22/mimetypes
mimeicon24dir = $(iconthemedir)/24x24/mimetypes
mimeicon32dir = $(iconthemedir)/32x32/mimetypes
mimeicon48dir = $(iconthemedir)/48x48/mimetypes
mimeiconscalabledir = $(iconthemedir)/scalable/mimetypes
statusicon16dir = $(iconthemedir)/16x16/status
statusicon22dir = $(iconthemedir)/22x22/status
statusicon32dir = $(iconthemedir)/32x32/status
statusicon48dir = $(iconthemedir)/48x48/status

dist_mimeicon16_DATA = \
	data/icons/16x16/mimetypes/application-x-remote-connection.png \
	data/icons/16x16/mimetypes/application-x-vnc.png
dist_mimeicon22_DATA = \
	data/icons/22x22/mimetypes/application-x-remote-connection.png \
	data/icons/22x22/mimetypes/application-x-vnc.png
dist_mimeicon24_DATA = \
	data/icons/24x24/mimetypes/application-x-remote-connection.png \
	data/icons/24x24/mimetypes/application-x-vnc.png
dist_mimeicon32_DATA = \
	data/icons/32x32/mimetypes/application-x-remote-connection.png \
	data/icons/32x32/mimetypes/application-x-vnc.png
dist_mimeicon48_DATA = \
	data/icons/48x48/mimetypes/application-x-remote-connection.png \
	data/icons/48x48/mimetypes/application-x-vnc.png
dist_mimeiconscalable_DATA = \
	data/icons/scalable/mimetypes/application-x-remote-connection.svg \
	data/icons/scalable/mimetypes/application-x-vnc.svg
dist_statusicon16_DATA = data/icons/16x16/status/view-minimize.png
dist_statusicon22_DATA = data/icons/22x22/status/view-minimize.png
dist_statusicon32_DATA = data/icons/32x32/status/view-minimize.png
dist_statusicon48_DATA = data/icons/48x48/status/view-minimize.png

update_icon_cache = gtk-update-icon-cache --ignore-theme-index --force

install-data-hook: install-update-icon-cache
uninstall-hook: uninstall-update-icon-cache

install-update-icon-cache:
	@$(POST_INSTALL)
	test -n "$(DESTDIR)" || $(update_icon_cache) "$(iconthemedir)"

uninstall-update-icon-cache:
	@$(POST_UNINSTALL)
	test -n "$(DESTDIR)" || $(update_icon_cache) "$(iconthemedir)"
dist_man_MANS = data/vinagre.1

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = data/vinagre-@VINAGRE_ABI_VERSION@.pc

dist_pkgdata_DATA = \
	data/vinagre-ui.xml \
	data/GNOME_VinagreApplet.xml \
	data/vinagre.ui \
	data/vinagre-fav-ui.xml

@INTLTOOL_DESKTOP_RULE@
desktopdir = $(datadir)/applications
desktop_in_files = data/vinagre.desktop.in data/vinagre-file.desktop.in
nodist_desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_XML_RULE@
mimedir = $(datadir)/mime/packages
nodist_mime_DATA = data/vinagre-mime.xml

gsettings_in_file = data/org.gnome.Vinagre.gschema.xml.in
gsettings_SCHEMAS = $(gsettings_in_file:.xml.in=.xml)
# @INTLTOOL_XML_NOMERGE_RULE@
# This rule is overriden by the above INTLTOOL_XML_RULE macro.
# Here is the workaround. Could break if intltools changes arguments.
$(gsettings_SCHEMAS): $(gsettings_in_file) $(INTLTOOL_MERGE)
	 LC_ALL=C $(INTLTOOL_MERGE) -x -u /tmp $< $@

@GSETTINGS_RULES@

if VINAGRE_HAVE_TELEPATHY_GLIB
clientfiledir = $(datarootdir)/telepathy/clients
dist_clientfile_DATA = data/Vinagre.client

servicedir = $(datadir)/dbus-1/services
service_in_file = data/org.freedesktop.Telepathy.Client.Vinagre.service.in
nodist_service_DATA = $(service_in_file:.in=)

$(nodist_service_DATA): $(service_in_file) Makefile
	$(AM_V_GEN)$(SED) -e "s|[@]bindir[@]|$(bindir)|" $< > $@
endif

if MAINTAINER_MODE
dist-hook: dist-changelog
else
dist-hook:
endif

dist-changelog:
	$(AM_V_at)if git --git-dir=$(top_srcdir)/.git --work-tree=$(top_srcdir) \
		log --no-merges --date=short --pretty='tformat:%cd  %an  <%ae>%n%n%s%n%n%b' f0cffbf192b629275f107322ea1dedfd69cf6fc6.. | \
	 $(SED)	-e '/^[12]...-[01].-[0123].  [^<>]*  <[^<>]*>$$/,/^$$/ b' \
		-e '/[^	 ]/,/^[	 ]*$$/ !d' \
		-e 's/^[	 ]*/	/' \
		-e 's/^[	 ]*$$//' >.ChangeLog.tmp; \
	then mv -f .ChangeLog.tmp "$(top_distdir)/ChangeLog"; \
	else rm -f .ChangeLog.tmp; exit 1; fi

dist_noinst_DATA = \
	$(noinst_vinagreh_headers) \
	$(noinst_plugin_headers) \
	ChangeLog.pre-git \
	data/vinagre.pc.in \
	data/vinagre-mime.xml.in \
	$(desktop_in_files) \
	$(gsettings_in_file) \
	$(service_in_file) \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	$(plugin_in_files) \
	vinagre.doap

CLEANFILES = \
	$(nodist_desktop_DATA) \
	$(nodist_mime_DATA) \
	$(gsettings_SCHEMAS) \
	$(nodist_pkgconfig_DATA) \
	$(nodist_service_DATA) \
	$(nodist_plugin_DATA)

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update

MAINTAINERCLEANFILES = \
	build-aux/compile \
	build-aux/config.guess \
	build-aux/config.sub \
	build-aux/depcomp \
	build-aux/gnome-doc-utils.make \
	build-aux/install-sh \
	build-aux/ltmain.sh \
	build-aux/missing \
	aclocal.m4 \
	$(gsettings_SCHEMAS:.xml=.valid) \
	config.h.in \
	mkinstalldirs \
	omf.make \
	xmldocs.make

.PHONY: install-update-icon-cache uninstall-update-icon-cache dist-changelog
