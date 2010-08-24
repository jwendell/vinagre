DIST_SUBDIRS =	\
  vnc		\
  ssh		\
  rdp		\
  reverse-vnc

SUBDIRS = vnc reverse-vnc

if SSH
SUBDIRS += ssh
endif

if RDP
SUBDIRS += rdp
endif

-include $(top_srcdir)/git.mk
