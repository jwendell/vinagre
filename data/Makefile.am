SUBDIRS = icons

man_MANS = vinagre.1

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = vinagre-@VINAGRE_API_VERSION@.pc

vinagre-@VINAGRE_API_VERSION@.pc: vinagre.pc
	cp vinagre.pc vinagre-@VINAGRE_API_VERSION@.pc

uidir = $(datadir)/vinagre
ui_DATA = vinagre-ui.xml GNOME_VinagreApplet.xml vinagre.ui

@INTLTOOL_DESKTOP_RULE@
desktopdir       = $(datadir)/applications
desktop_in_files = vinagre.desktop.in vinagre-file.desktop.in
desktop_DATA     = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_XML_RULE@
MIMEFILES = vinagre-mime.xml.in
mimedir = $(datadir)/mime/packages
mime_DATA = vinagre-mime.xml

schemasdir	 = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = vinagre.schemas.in
schemas_DATA 	 = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/data/$$p ; \
		done \
	fi
else
install-data-local:
endif

CLEANFILES = \
	$(desktop_DATA) \
	$(mime_DATA) \
	$(schemas_DATA) \
	$(pkgconfig_DATA)

EXTRA_DIST = \
	$(glade_DATA) \
	$(man_MANS) \
	$(ui_DATA) \
	$(desktop_in_files) \
	$(MIMEFILES) \
	$(schemas_in_files) \
	vinagre.pc.in \
	vinagre.schemas.in.in

if TELEPATHY
servicedir = $(datadir)/dbus-1/services
service_DATA = org.gnome.Empathy.StreamTubeHandler.rfb.service

org.gnome.Empathy.StreamTubeHandler.rfb.service: org.gnome.Empathy.StreamTubeHandler.rfb.service.in \
		Makefile
	@sed -e "s|[@]bindir[@]|$(bindir)|" $< > $@
endif

CLEANFILES += $(service_DATA)
EXTRA_DIST += org.gnome.Empathy.StreamTubeHandler.rfb.service.in

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
