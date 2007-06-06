gladedir   = $(datadir)/vinagre
glade_DATA = vinagre.glade

uidir = $(datadir)/vinagre
ui_DATA = vinagre-ui.xml

desktopdir       = $(datadir)/applications
desktop_in_files = vinagre.desktop.in
desktop_DATA     = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

CLEANFILES = \
	$(desktop_DATA)

EXTRA_DIST = \
	$(glade_DATA) \
	$(ui_DATA) \
	$(desktop_in_files)
