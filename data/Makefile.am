SUBDIRS = icons

man_MANS = vinagre.1

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = vinagre-@VINAGRE_API_VERSION@.pc

vinagre-@VINAGRE_API_VERSION@.pc: vinagre.pc
	@cp vinagre.pc vinagre-@VINAGRE_API_VERSION@.pc

uidir = $(datadir)/vinagre
ui_DATA = vinagre-ui.xml GNOME_VinagreApplet.xml vinagre.ui vinagre-fav-ui.xml

@INTLTOOL_DESKTOP_RULE@
desktopdir       = $(datadir)/applications
desktop_in_files = vinagre.desktop.in vinagre-file.desktop.in
desktop_DATA     = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_XML_RULE@
MIMEFILES = vinagre-mime.xml.in
mimedir = $(datadir)/mime/packages
mime_DATA = vinagre-mime.xml

gsettings_in_file = org.gnome.Vinagre.gschema.xml.in
gsettings_SCHEMAS = $(gsettings_in_file:.xml.in=.xml)
# @INTLTOOL_XML_NOMERGE_RULE@ # This rule is overriden by the above
                              # INTLTOOL_XML_RULE macro.
# Here is the workaround. Could break if intltools changes arguments.
$(gsettings_SCHEMAS): $(gsettings_in_file)
	$(INTLTOOL_MERGE) ; LC_ALL=C $(INTLTOOL_MERGE) -x -u /tmp $< [$]@

@GSETTINGS_RULES@

dist_noinst_DATA = \
	$(gsettings_in_file)

CLEANFILES = \
	$(desktop_DATA) \
	$(mime_DATA) \
	$(gsettings_SCHEMAS) \
	$(pkgconfig_DATA) \
	org.freedesktop.Telepathy.Client.Vinagre.service

EXTRA_DIST = \
	$(glade_DATA) \
	$(man_MANS) \
	$(ui_DATA) \
	$(desktop_in_files) \
	$(MIMEFILES) \
	$(gsettings_in_files) \
	vinagre.pc.in \
	Vinagre.client \
	org.freedesktop.Telepathy.Client.Vinagre.service.in

if TELEPATHY

clientfiledir = $(datarootdir)/telepathy/clients
clientfile_DATA = Vinagre.client

servicedir = $(datadir)/dbus-1/services
service_DATA = org.freedesktop.Telepathy.Client.Vinagre.service

org.freedesktop.Telepathy.Client.Vinagre.service: org.freedesktop.Telepathy.Client.Vinagre.service.in \
		Makefile
	$(AM_V_GEN)sed -e "s|[@]bindir[@]|$(bindir)|" $< > $@

endif

UPDATE_MIME = \
        if [ -f $(DESTDIR)$(datadir)/mime/packages/freedesktop.org.xml ] ; then \
                if which update-mime-database>/dev/null 2>&1; then \
                        update-mime-database $(DESTDIR)$(datadir)/mime; \
                fi \
        fi

UPDATE_DESKTOP = \
        if [ -f $(DESTDIR)$(datadir)/applications/defaults.list ] ; then \
                if which update-desktop-database>/dev/null 2>&1 ; then \
                        update-desktop-database; \
                fi \
        fi

install-data-hook:
	$(UPDATE_MIME)
	$(UPDATE_DESKTOP)

uninstall-hook:
	$(UPDATE_MIME)
	$(UPDATE_DESKTOP)

-include $(top_srcdir)/git.mk

MAINTAINERCLEANFILES = $(gsettings_SCHEMAS:.xml=.valid)
