NULL =

DIST_SUBDIRS =					\
	im-status				\
	rdp					\
	reverse-vnc				\
	spice					\
	ssh					\
	vnc					\
	$(NULL)

SUBDIRS =					\
	im-status				\
	reverse-vnc				\
	vnc					\
	$(NULL)

if VINAGRE_ENABLE_RDP
SUBDIRS += rdp
endif

if VINAGRE_ENABLE_SPICE
SUBDIRS += spice
endif

if VINAGRE_ENABLE_SSH
SUBDIRS += ssh
endif

-include $(top_srcdir)/git.mk
