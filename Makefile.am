ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
SUBDIRS = vinagre help po data/icons plugins

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper --without-panelapplet

dist_man_MANS = data/vinagre.1

pkgconfigdir = $(libdir)/pkgconfig
nodist_pkgconfig_DATA = data/vinagre-@VINAGRE_API_VERSION@.pc

data/vinagre-@VINAGRE_API_VERSION@.pc: data/vinagre.pc
	$(AM_V_GEN)cp $< $@

uidir = $(pkgdatadir)
dist_ui_DATA = \
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
