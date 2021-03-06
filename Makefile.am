ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
SUBDIRS = help po

DISTCHECK_CONFIGURE_FLAGS = \
	--disable-scrollkeeper \
	--enable-compile-warnings=maximum

vinagre_vinagre_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_builddir)/vinagre \
	-I$(top_srcdir) \
	-I$(top_srcdir)/vinagre \
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
	$(VNC_CFLAGS) \
	$(WARN_CFLAGS) \
	$(DISABLE_DEPRECATED) \
	-include config.h

vinagre_vinagre_LDADD = \
	$(VINAGRE_LIBS) \
	$(VNC_LIBS)

AM_VALAFLAGS = \
	--vapidir vinagre/vapi \
	--pkg config \
	--pkg gtk+-3.0

bin_PROGRAMS = vinagre/vinagre

noinst_vinagreh_headers = \
	vinagre/if/ifaddrs.h \
	vinagre/view/autoDrawer.h \
	vinagre/view/drawer.h \
	vinagre/view/ovBox.h \
	vinagre/vinagre-window-private.h \
	vinagre/vinagre-bookmarks-entry.h \
	vinagre/vinagre-bookmarks.h \
	vinagre/vinagre-bookmarks-migration.h \
	vinagre/vinagre-bookmarks-tree.h \
	vinagre/vinagre-bookmarks-ui.h \
	vinagre/vinagre-cache-prefs.h \
	vinagre/vinagre-commands.h \
	vinagre/vinagre-connect.h \
	vinagre/vinagre-connection.h \
	vinagre/vinagre-debug.h \
	vinagre/vinagre-dnd.h \
	vinagre/vinagre-notebook.h \
	vinagre/vinagre-options.h \
	vinagre/vinagre-plugins-engine.h \
	vinagre/vinagre-prefs.h \
	vinagre/vinagre-protocol.h \
	vinagre/vinagre-reverse-vnc-listener.h \
	vinagre/vinagre-reverse-vnc-listener-dialog.h \
	vinagre/vinagre-static-extension.h \
	vinagre/vinagre-tab.h \
	vinagre/vinagre-ui.h \
	vinagre/vinagre-window.h \
	vinagre/vinagre-ssh.h

vinagre_vala_sources = \
	vinagre/vinagre-dirs.vala \
	vinagre/vinagre-utils.vala

vinagre_vinagre_SOURCES = \
	vinagre/vinagre-main.c \
	vinagre/vinagre-options.c \
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
	vinagre/vinagre-notebook.c \
	vinagre/vinagre-prefs.c \
	vinagre/vinagre-reverse-vnc-listener.c \
	vinagre/vinagre-reverse-vnc-listener-dialog.c \
	vinagre/vinagre-static-extension.c \
	vinagre/vinagre-tab.c \
	vinagre/vinagre-window.c \
	vinagre/vinagre-ssh.c \
	vinagre/vinagre-cache-prefs.c \
	vinagre/vinagre-protocol.c \
	vinagre/vinagre-plugins-engine.c \
	plugins/vnc/vinagre-vnc-plugin.c \
	plugins/vnc/vinagre-vnc-connection.c \
	plugins/vnc/vinagre-vnc-tab.c \
	plugins/vnc/vinagre-vnc-tunnel.c \
	$(vinagre_vala_sources:.vala=.c)

if VINAGRE_HAVE_SELF_IFADDRS
vinagre_vinagre_SOURCES += \
       vinagre/if/getifaddrs.c
endif

if VINAGRE_PTY_OPEN
noinst_vinagreh_headers += \
	vinagre/pty_open.h
vinagre_vinagre_SOURCES += \
	vinagre/pty_open.c
endif

if VINAGRE_HAVE_AVAHI
vinagre_vinagre_SOURCES += \
	vinagre/vinagre-mdns.c
noinst_vinagreh_headers += \
	vinagre/vinagre-mdns.h
endif

# Telepathy stuff
if VINAGRE_HAVE_TELEPATHY_GLIB
vinagre_vinagre_SOURCES += \
	vinagre/vinagre-tubes-manager.c \
	vinagre/vinagre-tube-handler.c
noinst_vinagreh_headers += \
	vinagre/vinagre-tubes-manager.h \
	vinagre/vinagre-tube-handler.h
endif

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
	plugins/vnc/vinagre-vnc-plugin.h \
	plugins/vnc/vinagre-vnc-connection.h \
	plugins/vnc/vinagre-vnc-tab.h \
	plugins/vnc/vinagre-vnc-tunnel.h

if VINAGRE_ENABLE_RDP
vinagre_vinagre_SOURCES += \
	plugins/rdp/vinagre-rdp-plugin.c \
	plugins/rdp/vinagre-rdp-connection.c \
	plugins/rdp/vinagre-rdp-tab.c
endif

if VINAGRE_ENABLE_SPICE
vinagre_vinagre_CPPFLAGS += \
	$(SPICE_CFLAGS) \
	-I$(top_srcdir)/plugins/spice

vinagre_vinagre_SOURCES += \
	plugins/spice/vinagre-spice-plugin.c \
	plugins/spice/vinagre-spice-connection.c \
	plugins/spice/vinagre-spice-tab.c \
	plugins/spice/vinagre-spice-tunnel.c

vinagre_vinagre_LDADD += $(SPICE_LIBS)
endif

if VINAGRE_ENABLE_SSH
vinagre_vinagre_SOURCES += \
	plugins/ssh/vinagre-ssh-plugin.c \
	plugins/ssh/vinagre-ssh-connection.c \
	plugins/ssh/vinagre-ssh-tab.c
endif

# Ensure vinagre-vala.h is available immediately since C sources #include it
BUILT_SOURCES = \
	vinagre/vinagre-vala.h

# We have to write an explicit rule instead of just adding --header AM_VALAFLAGS
# or VALAFLAGS because automake does not support per-target VALAFLAGS, so if in
# the future we use a .vala file in a plugin, vinagre-vala.h will get clobbered.
vinagre_vala.stamp: $(vinagre_vala_sources)
	$(AM_V_VALAC)$(VALAC) $(AM_VALAFLAGS) $(VALAFLAGS) --header vinagre/vinagre-vala.h -C $(vinagre_vala_sources)
	$(AM_V_at)touch $@

# Automake doesn't have an implicit rule to generate headers from vala
vinagre/vinagre-vala.h $(vinagre_vala_sources:.vala=.c): $(srcdir)/vinagre_vala.stamp
	@if test -f $@; then :; else \
	  rm -f $(srcdir)/vinagre_vala.stamp; \
	  $(am__cd) $(srcdir) && $(MAKE) $(AM_MAKEFLAGS) vinagre_vala.stamp; \
	fi

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

dist_pkgdata_DATA = \
	data/vinagre-ui.xml \
	data/vinagre.ui

@INTLTOOL_DESKTOP_RULE@
desktopdir = $(datadir)/applications
desktop_in_files = data/vinagre.desktop.in data/vinagre-file.desktop.in
nodist_desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_XML_RULE@
mimedir = $(datadir)/mime/packages
nodist_mime_DATA = data/vinagre-mime.xml

gsettings_SCHEMAS = data/org.gnome.Vinagre.gschema.xml

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

convertdir = $(datadir)/GConf/gsettings
dist_convert_DATA = data/org.gnome.Vinagre.convert

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
	$(vinagre_vala_sources) \
	vinagre/vinagre-vala.h \
	vinagre_vala.stamp \
	$(noinst_plugin_headers) \
	ChangeLog.pre-git \
	data/vinagre-mime.xml.in \
	$(desktop_in_files) \
	$(gsettings_SCHEMAS) \
	$(service_in_file) \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	vinagre/vapi/config.vapi \
	vinagre.doap

CLEANFILES = \
	$(nodist_desktop_DATA) \
	$(nodist_mime_DATA) \
	$(nodist_pkgconfig_DATA) \
	$(nodist_service_DATA)

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update

MAINTAINERCLEANFILES = \
	build-aux/compile \
	build-aux/config.guess \
	build-aux/config.sub \
	build-aux/depcomp \
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
