DIST_SUBDIRS =	\
  vnc		\
  ssh		\
  rdp		\
  reverse-vnc	\
  im-status

SUBDIRS =	\
  vnc		\
  reverse-vnc	\
  im-status

if SSH
SUBDIRS += ssh
endif

if RDP
SUBDIRS += rdp
endif

-include $(top_srcdir)/git.mk
