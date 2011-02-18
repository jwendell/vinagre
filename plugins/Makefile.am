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

if RDP
SUBDIRS += rdp
endif

if SPICE
SUBDIRS += spice
endif

if SSH
SUBDIRS += ssh
endif

-include $(top_srcdir)/git.mk
