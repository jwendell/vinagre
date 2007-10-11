gladedir   = $(datadir)/vinagre
glade_DATA = vinagre.glade

uidir = $(datadir)/vinagre
ui_DATA = vinagre-ui.xml

iconsdir = $(datadir)/icons/hicolor/scalable/apps
icons_DATA = vinagre.svg

@INTLTOOL_DESKTOP_RULE@

desktopdir       = $(datadir)/applications
desktop_in_files = vinagre.desktop.in
desktop_DATA     = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_XML_RULE@

MIMEFILES = vinagre-mime.xml.in
mimedir = $(datadir)/mime/packages
mime_DATA = vinagre-mime.xml

CLEANFILES = \
	$(desktop_DATA)

EXTRA_DIST = \
	$(glade_DATA) \
	$(ui_DATA) \
	$(desktop_in_files) \
	$(mime_DATA) \
	$(icons_DATA)

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

UPDATE_ICON =  gtk-update-icon-cache -f -t $(DESTDIR)$(datadir)/icons/hicolor

install-data-hook: 
	$(UPDATE_MIME)
	$(UPDATE_DESKTOP)
	mkdir -p  $(DESTDIR)$(datadir)/icons/hicolor/scalable/mimetypes/
	ln -s $(DESTDIR)$(datadir)/icons/hicolor/scalable/apps/vinagre.svg $(DESTDIR)$(datadir)/icons/hicolor/scalable/mimetypes/application-x-vnc.svg
	$(UPDATE_ICON)

uninstall-hook: 
	$(UPDATE_MIME)
	$(UPDATE_DESKTOP)
	rm -f $(DESTDIR)$(datadir)/icons/hicolor/scalable/mimetypes/application-x-vnc.svg
	$(UPDATE_ICON)
