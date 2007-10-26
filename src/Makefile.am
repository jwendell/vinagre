INCLUDES = 						\
	-DBINDIR=\"$(bindir)\"			 	\
	-DDATADIR=\"$(datadir)\"			\
	-DPREFIX=\""$(prefix)"\" 			\
	-DSYSCONFDIR=\""$(sysconfdir)"\" 		\
	-DLIBDIR=\""$(libdir)"\" 			\
	-DVINAGRE_DATADIR=\""$(datadir)/vinagre"\"	\
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\"	\
	$(VINAGRE_CFLAGS)				\
	$(AVAHI_CFLAGS)

bin_PROGRAMS = vinagre

vinagre_SOURCES = \
	vinagre-commands.c vinagre-commands.h		\
	vinagre-connect.c vinagre-connect.h		\
	vinagre-connection.c vinagre-connection.h	\
	vinagre-fav.c vinagre-fav.h			\
	vinagre-bookmarks.c vinagre-bookmarks.h		\
	vinagre-main.c 			\
	vinagre-notebook.c vinagre-notebook.h		\
	vinagre-prefs-manager.c	vinagre-prefs-manager.h	\
	vinagre-tab.c vinagre-tab.h			\
	vinagre-utils.c vinagre-utils.h			\
	vinagre-window.c vinagre-window.h		\
	vinagre-ui.h vinagre-window-private.h

vinagre_LDADD = \
	$(VINAGRE_LIBS)		\
	$(GTKVNC_LIBS)		\
	$(AVAHI_LIBS)

