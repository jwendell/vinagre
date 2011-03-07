ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
SUBDIRS = vinagre help po plugins

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper --without-panelapplet

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
nodist_pkgconfig_DATA = data/vinagre-@VINAGRE_API_VERSION@.pc

data/vinagre-@VINAGRE_API_VERSION@.pc: data/vinagre.pc
	$(AM_V_GEN)cp $< $@

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
	intltool-update.in

CLEANFILES = \
	$(nodist_desktop_DATA) \
	$(nodist_mime_DATA) \
	$(gsettings_SCHEMAS) \
	$(nodist_pkgconfig_DATA) \
	data/org.freedesktop.Telepathy.Client.Vinagre.service

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

.PHONY: install-update-icon-cache uninstall-update-icon-cache
