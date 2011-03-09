ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
SUBDIRS = help po plugins

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper --without-panelapplet

AM_CPPFLAGS = \
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
	$(INTROSPECTION_CFLAGS)

bin_PROGRAMS = vinagre/vinagre

vinagre_vinagre_SOURCES = \
	vinagre/vinagre-main.c \
	vinagre/vinagre-options.h \
	vinagre/vinagre-options.c

vinagre_vinagre_LDADD = \
	libvinagre.la \
	$(VINAGRE_LIBS) \
	$(GTKVNC_LIBS) \
	$(INTROSPECTION_LIBS)

vinagrehdir = $(includedir)/vinagre-@VINAGRE_API_VERSION@/vinagre

noinst_vinagreh_headers = \
	vinagre/view/autoDrawer.h \
	vinagre/view/drawer.h \
	vinagre/view/ovBox.h \
	vinagre/vinagre-enums.h \
	vinagre/vinagre-marshal.h \
	vinagre/vinagre-plugin-dialog.h \
	vinagre/vinagre-window-private.h \
	vinagre/pty_open.h

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
	vinagre/vinagre-tab.h \
	vinagre/vinagre-ui.h \
	vinagre/vinagre-utils.h \
	vinagre/vinagre-window.h \
	vinagre/vinagre-dnd.h \
	vinagre/vinagre-ssh.h \
	vinagre/vinagre-cache-prefs.h \
	vinagre/vinagre-protocol.h \
	vinagre/vinagre-plugins-engine.h

INTROSPECTION_GIR_INCLUDES = \
	Gtk-3.0 \
	libxml2-2.0 \
	Peas-1.0 \
	PeasGtk-1.0

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
	vinagre/vinagre-plugin-dialog.c \
	vinagre/vinagre-prefs.c \
	vinagre/vinagre-tab.c \
	vinagre/vinagre-utils.c \
	vinagre/vinagre-window.c \
	vinagre/pty_open.c \
	vinagre/vinagre-ssh.c \
	vinagre/vinagre-cache-prefs.c \
	vinagre/vinagre-protocol.c \
	vinagre/vinagre-plugins-engine.c

libvinagre_la_SOURCES = \
	vinagre/vinagre-enums.c \
	vinagre/vinagre-marshal.c \
	$(handwritten_sources) \
	$(noinst_vinagreh_headers) \
	$(vinagreh_HEADERS)

if VINAGRE_HAVE_AVAHI
libvinagre_la_SOURCES += \
	vinagre/vinagre-mdns.h \
	vinagre/vinagre-mdns.c
endif

libvinagre_la_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"

libvinagre_la_LIBADD = $(VINAGRE_LIBS)

if VINAGRE_HAVE_PANELAPPLET

# Bonobo .server
serverdir = $(libdir)/bonobo/servers
server_in_files = GNOME_VinagreApplet.server.in
nodist_server_DATA = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	$(AM_V_GEN)$(SED) -e "s|\@LIBEXECDIR\@|$(libexecdir)|" -e "s|\@PACKAGE_VERSION\@|$(PACKAGE_VERSION)|" $< > $@

@INTLTOOL_SERVER_RULE@

# The applet binary
libexec_PROGRAMS = vinagre-applet

vinagre_applet_CFLAGS = \
	$(PANELAPPLET_CFLAGS)

vinagre_applet_SOURCES = \
	vinagre/vinagre-applet.c \
	vinagre/vinagre-bookmarks.h \
	vinagre/vinagre-bookmarks.c \
	vinagre/vinagre-bookmarks-ui.h \
	vinagre/vinagre-bookmarks-ui.c \
	vinagre/vinagre-connection.h \
	vinagre/vinagre-connection.c \
	vinagre/vinagre-utils.h \
	vinagre/vinagre-utils.c \
	vinagre/vinagre-enums.h \
	vinagre/vinagre-enums.c \
	vinagre/vinagre-bookmarks-entry.h \
	vinagre/vinagre-bookmarks-entry.c \
	vinagre/vinagre-bookmarks-tree.h \
	vinagre/vinagre-bookmarks-tree.c \
	vinagre/vinagre-bookmarks-migration.h \
	vinagre/vinagre-bookmarks-migration.c \
	vinagre/vinagre-plugin-dialog.h \
	vinagre/vinagre-plugin-dialog.c \
	vinagre/vinagre-debug.h \
	vinagre/vinagre-debug.c \
	vinagre/vinagre-window.h \
	vinagre/vinagre-window.c \
	vinagre/vinagre-commands.h \
	vinagre/vinagre-commands.c \
	vinagre/vinagre-notebook.h \
	vinagre/vinagre-notebook.c \
	vinagre/vinagre-tab.h \
	vinagre/vinagre-tab.c \
	vinagre/vinagre-prefs.h \
	vinagre/vinagre-prefs.c \
	vinagre/vinagre-dirs.h \
	vinagre/vinagre-dirs.c \
	vinagre/vinagre-fav.h \
	vinagre/vinagre-fav.c \
	vinagre/vinagre-connect.h \
	vinagre/vinagre-connect.c \
	vinagre/vinagre-marshal.h \
	vinagre/vinagre-marshal.c \
	vinagre/pty_open.h \
	vinagre/pty_open.c \
	vinagre/vinagre-ssh.h \
	vinagre/vinagre-ssh.c \
	vinagre/vinagre-cache-prefs.h \
	vinagre/vinagre-cache-prefs.c \
	vinagre/vinagre-protocol.h \
	vinagre/vinagre-protocol.c \
	vinagre/vinagre-plugins-engine.h \
	vinagre/vinagre-plugins-engine.c

if VINAGRE_HAVE_AVAHI
vinagre_applet_SOURCES += \
	vinagre/vinagre-mdns.h \
	vinagre/vinagre-mdns.c
endif

vinagre_applet_LDADD = \
	vinagre/view/libview.la \
	$(PANELAPPLET_LIBS) \
	$(VINAGRE_LIBS)

vinagre_applet_LDFLAGS = -export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"

endif

# Autogenerated stuff
vinagre_enum_headers = $(top_srcdir)/vinagre/vinagre-connection.h

vinagre/vinagre-enums.c: $(vinagre_enum_headers)
	$(AM_V_GEN)$(GLIB_MKENUMS) --fhead "#include <glib-object.h>\n" \
			--fhead "#include \"vinagre-enums.h\"\n\n" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--fprod "\n#include \"@filename@\"\n" \
			--vhead "static const G@Type@Value _@enum_name@_values[] = {" \
			--vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "  { 0, NULL, NULL }\n};\n\n" \
			--vtail "GType\n@enum_name@_get_type (void)\n{\n" \
			--vtail "  static GType type = 0;\n\n" \
			--vtail "  if (!type)\n" \
			--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
			--vtail "  return type;\n}\n\n" \
		$(vinagre_enum_headers) > $@

vinagre/vinagre-enums.h: $(vinagre_enum_headers)
	$(AM_V_GEN)$(GLIB_MKENUMS) --fhead "#ifndef __VINAGRE_ENUMS_H__\n" \
			--fhead "#define __VINAGRE_ENUMS_H__ 1\n\n" \
			--fhead "G_BEGIN_DECLS\n\n" \
			--ftail "G_END_DECLS\n\n" \
			--ftail "#endif /* __VINAGRE_ENUMS_H__ */\n" \
			--fprod "\n/* --- @filename@ --- */" \
			--eprod "#define VINAGRE_TYPE_@ENUMSHORT@ @enum_name@_get_type()\n" \
			--eprod "GType @enum_name@_get_type (void);\n" \
		$(vinagre_enum_headers) >  $@

vinagre/vinagre-marshal.list: $(handwritten_sources) Makefile.am
	$(AM_V_GEN)( cd $(srcdir) && \
	$(SED) -n -e 's/.*vinagre_marshal_\([[:upper:][:digit:]]*__[[:upper:][:digit:]_]*\).*/\1/p' \
	$(handwritten_sources) ) \
	| $(SED) -e 's/__/:/' -e 'y/_/,/' | sort -u > $@.tmp && \
	if cmp -s $@.tmp $@; then \
		rm $@.tmp; \
	else \
		mv $@.tmp $@; \
	fi

%-marshal.h: %-marshal.list Makefile
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --header --prefix=_$(subst -,_,$*)_marshal $< > $*-marshal.h

%-marshal.c: %-marshal.list Makefile
	$(AM_V_GEN)echo "#include \"vinagre-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) --body --prefix=_$(subst -,_,$*)_marshal $< >> $*-marshal.c

BUILT_SOURCES = \
	vinagre/vinagre-enums.c \
	vinagre/vinagre-enums.h \
	vinagre/vinagre-marshal.list \
	vinagre/vinagre-marshal.c \
	vinagre/vinagre-marshal.h

# Telepathy stuff
if VINAGRE_HAVE_TELEPATHY_GLIB
handwritten_sources += \
	vinagre/vinagre-tubes-manager.c \
	vinagre/vinagre-tubes-manager.h \
	vinagre/vinagre-tube-handler.c \
	vinagre/vinagre-tube-handler.h
INTROSPECTION_GIR_INCLUDES += TelepathyGLib-0.12
endif

CLEANFILES =

# Introspection
if VINAGRE_HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = Vinagre-3.0.gir

Vinagre-3.0.gir: vinagre
INTROSPECTION_SCANNER_ARGS = $(AM_CPPFLAGS) $(VINAGRE_CFLAGS) -I$(top_srcdir) --warn-all
Vinagre_3_0_gir_LIBS = $(builddir)/libvinagre.la
Vinagre_3_0_gir_FILES = $(vinagreh_HEADERS) $(filter-out pty_open.h vinagre-plugin-dialog.h, $(noinst_vinagreh_headers)) $(handwritten_sources)
Vinagre_3_0_gir_INCLUDES = $(INTROSPECTION_GIR_INCLUDES)

girdir = $(pkgdatadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/vinagre-1/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += \
	$(gir_DATA) \
	$(typelib_DATA)

endif

iconthemedir = $(datadir)/icons/hicolor
appicon16dir = $(iconthemedir)/16x16/apps
appicon22dir = $(iconthemedir)/22x22/apps
appicon24dir = $(iconthemedir)/24x24/apps
appicon32dir = $(iconthemedir)/32x32/apps
appicon48dir = $(iconthemedir)/48x48/apps
appiconscalabledir = $(iconthemedir)/scalable/apps
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

dist_appicon16_DATA = data/icons/16x16/apps/vinagre.png
dist_appicon22_DATA = data/icons/22x22/apps/vinagre.png
dist_appicon24_DATA = data/icons/24x24/apps/vinagre.png
dist_appicon32_DATA = data/icons/32x32/apps/vinagre.png
dist_appicon48_DATA = data/icons/48x48/apps/vinagre.png
dist_appiconscalable_DATA = data/icons/scalable/apps/vinagre.svg
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
$(gsettings_SCHEMAS): $(gsettings_in_file)
	$(INTLTOOL_MERGE) ; LC_ALL=C $(INTLTOOL_MERGE) -x -u /tmp $< $@

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

dist_noinst_DATA = \
	data/icons/hicolor_apps_16x16_vinagre.svg \
	data/icons/hicolor_apps_16x16_vinagre.xcf \
	data/icons/hicolor_apps_22x22_vinagre.svg \
	data/icons/hicolor_apps_32x32_vinagre.svg \
	data/vinagre.pc.in \
	data/vinagre-mime.xml.in \
	$(desktop_in_files) \
	$(gsettings_in_file) \
	$(service_in_file) \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	vinagre/GNOME_VinagreApplet.server.in.in \
	vinagre/vinagre-marshal.list \
	vinagre.doap

CLEANFILES += \
	$(nodist_desktop_DATA) \
	$(nodist_mime_DATA) \
	$(gsettings_SCHEMAS) \
	$(nodist_pkgconfig_DATA) \
	data/org.freedesktop.Telepathy.Client.Vinagre.service

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update \
	$(BUILT_SOURCES) \
	$(server_in_files) \
	$(server_DATA)

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

.PHONY: install-update-icon-cache uninstall-update-icon-cache
