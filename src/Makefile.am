SUBDIRS = gtk-vnc

INCLUDES = 								\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\"				\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"				\
	-DVINAGRE_GLADEDIR=\""$(datadir)/vinagre"\"			\
	$(VINAGRE_CFLAGS)						\
	$(AVAHI_CFLAGS)

bin_PROGRAMS = vinagre

if VINAGRE_AVAHI
VINAGRE_AVAHI_SRC =	\
	avahi-ui.h	\
	avahi-ui.c
endif

vinagre_SOURCES = \
	vinagre-commands.c vinagre-commands.h		\
	vinagre-connect.c vinagre-connect.h		\
	vinagre-connection.c vinagre-connection.h	\
	vinagre-fav.c vinagre-fav.h			\
	vinagre-favorites.c vinagre-favorites.h		\
	vinagre-main.c vinagre-main.h			\
	vinagre-notebook.c vinagre-notebook.h		\
	vinagre-prefs-manager.c	vinagre-prefs-manager.h	\
	vinagre-socket.c vinagre-socket.h		\
	vinagre-tab.c vinagre-tab.h			\
	vinagre-utils.c vinagre-utils.h			\
	vinagre-window.c vinagre-window.h		\
	vinagre-ui.h vinagre-window-private.h		\
	$(VINAGRE_AVAHI_SRC)

vinagre_LDADD = \
	$(VINAGRE_LIBS)		\
	gtk-vnc/libgtkvnc.la	\
	$(GTKVNC_LIBS)		\
	$(AVAHI_LIBS)

