# Vinagre autoconf

AC_PREREQ([2.64])
AC_INIT([vinagre],[2.91.9],[https://bugzilla.gnome.org/enter_bug.cgi?product=vinagre],[vinagre],[http://projects.gnome.org/vinagre/])
AC_CONFIG_SRCDIR([vinagre/vinagre-main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([sys/uio.h sys/param.h sys/un.h arpa/inet.h stropts.h fcntl.h limits.h locale.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h termios.h unistd.h utmp.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 memset putenv select setlocale socket strchr strerror strstr getpt posix_openpt grantpt unlockpt ptsname ptsname_r])

AM_INIT_AUTOMAKE([1.10 -Wno-portability])
LT_PREREQ([2.2.6])
AC_CONFIG_MACRO_DIR([m4])

GNOME_COMPILE_WARNINGS([maximum])
GNOME_MAINTAINER_MODE_DEFINES

VINAGRE_API_VERSION=1.0
AC_SUBST(VINAGRE_API_VERSION)

AM_MAINTAINER_MODE

# Use the silent-rules feature when possible.
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

dnl make sure we keep ACLOCAL_FLAGS around for maintainer builds to work
AC_SUBST(ACLOCAL_AMFLAGS, "\${ACLOCAL_FLAGS}")

AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CC
AC_HEADER_STDC

LT_INIT([disable-static])
AM_PROG_CC_C_O

#*******************************************************************************
# Internationalization
#*******************************************************************************
GETTEXT_PACKAGE=vinagre
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

IT_PROG_INTLTOOL([0.40.0])
AM_GLIB_GNU_GETTEXT

GNOME_DOC_INIT

#*******************************************************************************
# Check required GNOME libraries
#*******************************************************************************

GLIB_REQUIRED=2.25.11
GTK_REQUIRED=2.99.3
GTHREAD_REQUIRED=2.0.0
GTK_VNC_REQUIRED=0.4.3
SPICE_GTK_REQUIRED=0.5
AVAHI_REQUIRED=0.6.26
PANEL_REQUIRED_VERSION=2.0.0
TELEPATHY_GLIB_REQUIRED=0.11.6
XML_REQUIRED=2.6.31
VTE_REQUIRED=0.20
PEAS_REQUIRED=0.7.2

# Check for avahi
AC_ARG_ENABLE(avahi,
              AS_HELP_STRING([--disable-avahi],[Do not use avahi for auto-discover machines [default=no]]),
              enable_avahi=${enableval},
              enable_avahi=yes)

if test "x$enable_avahi" = "xyes" ; then
  PKG_CHECK_MODULES(AVAHI,\
  avahi-ui-gtk3 >= $AVAHI_REQUIRED\
  avahi-gobject >= $AVAHI_REQUIRED)
  AC_SUBST(AVAHI_CFLAGS)
  AC_SUBST(AVAHI_LIBS)
  AC_DEFINE([VINAGRE_ENABLE_AVAHI],[1],[We want avahi support])
fi
AM_CONDITIONAL(AVAHI, test "x$enable_avahi" = "xyes")

PKG_CHECK_MODULES(VINAGRE, \
  glib-2.0 >= $GLIB_REQUIRED \
  gio-2.0 >= $GLIB_REQUIRED \
  gio-unix-2.0 >= $GLIB_REQUIRED \
  gtk+-3.0 >= $GTK_REQUIRED \
  gthread-2.0 >= $GTHREAD_REQUIRED \
  gnome-keyring-1 \
  libxml-2.0 >= $XML_REQUIRED \
  libpeas-1.0 >= $PEAS_REQUIRED \
  libpeas-gtk-1.0 >= $PEAS_REQUIRED
)
AC_SUBST(VINAGRE_CFLAGS)
AC_SUBST(VINAGRE_LIBS)

PKG_CHECK_MODULES(VNC, \
  gtk-vnc-2.0 >= $GTK_VNC_REQUIRED \
  gtk+-3.0 >= $GTK_REQUIRED \
  libxml-2.0 >= $XML_REQUIRED
)
AC_SUBST(VNC_CFLAGS)
AC_SUBST(VNC_LIBS)

dnl ****************************
dnl *** Checks for pty stuff ***
dnl ****************************

# Pull in the right libraries for various functions which might not be
# bundled into an exploded libc.
AC_CHECK_FUNC(socketpair,[have_socketpair=1],AC_CHECK_LIB(socket,socketpair,[have_socketpair=1; LIBS="$LIBS -lsocket"]))
if test x$have_socketpair = x1 ; then
	AC_DEFINE(HAVE_SOCKETPAIR,1,[Define if you have the socketpair function.])
fi

AC_SEARCH_LIBS(login_tty, util, [AC_DEFINE([HAVE_LOGIN_TTY],[],[Whether login_tty is available])])

dnl ****************************
dnl *** Checks for SSH stuff ***
dnl ****************************

AC_PATH_PROG(SSH_PROGRAM, ssh, "ssh")

AC_ARG_ENABLE(ssh,
              AS_HELP_STRING([--enable-ssh],
                             [Enable SSH plugin (default=no)]),
              enable_ssh=${enableval},
              enable_ssh=no)

if test "x$enable_ssh" = "xyes" ; then
  PKG_CHECK_MODULES(SSH,\
  vte-2.90 >= $VTE_REQUIRED \
  libxml-2.0 >= $XML_REQUIRED)
  AC_SUBST(VTE_CFLAGS)
  AC_SUBST(VTE_LIBS)
  AC_DEFINE([VINAGRE_ENABLE_SSH],[1],[We want ssh support])
fi
AM_CONDITIONAL(SSH, test "x$enable_ssh" = "xyes")

dnl ****************************
dnl *** Checks for RDP stuff ***
dnl ****************************

AC_ARG_ENABLE(rdp,
              AS_HELP_STRING([--enable-rdp],
                             [Enable RDP plugin (default=yes)]),
              enable_rdp=${enableval},
              enable_rdp=yes)

if test "x$enable_rdp" = "xyes" ; then
  AC_PATH_PROG(RDESKTOP_PROGRAM, rdesktop, "rdesktop")
  PKG_CHECK_MODULES(RDP,\
  libxml-2.0 >= $XML_REQUIRED)
  AC_SUBST(RDP_CFLAGS)
  AC_SUBST(RDP_LIBS)
  AC_DEFINE([VINAGRE_ENABLE_RDP],[1],[We want rdp support])
fi
AM_CONDITIONAL(RDP, test "x$enable_rdp" = "xyes")

# Check for telepathy
AC_ARG_ENABLE(telepathy,
              AS_HELP_STRING([--enable-telepathy=@<:@no/yes/auto@:>@],
                             [Enable telepathy checking]), ,
                             enable_telepathy=auto)

if test "x$enable_telepathy" != "xno"; then
   PKG_CHECK_MODULES(TELEPATHY,
   [
      telepathy-glib >= $TELEPATHY_GLIB_REQUIRED
   ], have_telepathy="yes", have_telepathy="no")
   if test "x$have_telepathy" = "xyes"; then
      AC_SUBST(TELEPATHY_CFLAGS)
      AC_SUBST(TELEPATHY_LIBS)
      AC_DEFINE(HAVE_TELEPATHY, [], [Define if you have telepathy])
   fi
else
   have_telepathy=no
fi

if test "x$enable_telepathy" = "xyes" -a "x$have_telepathy" != "xyes"; then
   AC_MSG_ERROR([Couldn't find telepathy dependencies.])
fi

AM_CONDITIONAL(TELEPATHY, test "x$have_telepathy" = "xyes")

# Enable/disable gnome-panel applet
AC_ARG_ENABLE(applet,
              AS_HELP_STRING([--disable-applet], [Do not build the panel applet @<:@default=enabled@:>@]),
              enable_applet=$enableval,
              enable_applet=yes)

if test "x$enable_applet" = "xyes";then
   PKG_CHECK_MODULES(APPLET, libpanelapplet-2.0 >= $PANEL_REQUIRED_VERSION)
   AC_SUBST(APPLET_CFLAGS)
   AC_SUBST(APPLET_LIBS)
fi
AM_CONDITIONAL(APPLET, test "x$enable_applet" = "xyes")

# Check for spice
AC_ARG_ENABLE(spice,
              AS_HELP_STRING([--enable-spice],
                             [Enable SPICE plugin (default=yes)]),
              enable_spice=${enableval},
              enable_spice=no)

if test "x$enable_spice" = "xyes" ; then
  PKG_CHECK_MODULES(SPICE,\
  spice-client-gtk-3.0 >= $SPICE_GTK_REQUIRED)
  AC_SUBST(SPICE_CFLAGS)
  AC_SUBST(SPICE_LIBS)
  AC_DEFINE([VINAGRE_ENABLE_SPICE],[1],[We want spice support])
fi
AM_CONDITIONAL(SPICE, test "x$enable_spice" = "xyes")

GLIB_GSETTINGS

dnl Check for bind in libsocket (needed on Solaris)
AC_CHECK_LIB(socket, bind)

dnl define a MAINT-like variable REBUILD which is set
dnl if Perl, so autogenerated sources can be rebuilt
AC_PATH_PROGS(PERL, perl5 perl)
AC_ARG_ENABLE(rebuilds, [  --disable-rebuilds      disable all source autogeneration rules],,enable_rebuilds=yes)
REBUILD=\#
if test "x$enable_rebuilds" = "xyes" && \
     test -n "$PERL" && \
     $PERL -e 'exit !($] >= 5.002)' > /dev/null 2>&1 ; then
  REBUILD=
fi
AC_SUBST(REBUILD)

# Plugins
#*******************************************************************************
#
dnl ****************************
dnl *** Checks for ifaddrs.h ***
dnl ****************************
AC_CHECK_HEADER(ifaddrs.h)
AM_CONDITIONAL(SELF_IFADDRS, test "x$ac_cv_header_ifaddrs_h" != "xyes")

dnl RFC2553 introduce sockaddr_storage as ifa_addr member in ifaddrs structure
dnl Not all distros follow this.
if test "x$ac_cv_header_ifaddrs_h" = "xyes"; then
        AC_DEFINE(HAVE_IFADDRS_H, [1], [Define if we have system ifaddrs.h])
        AC_TRY_COMPILE([
          #include <ifaddrs.h>
          #include <net/if.h>
          ],[
          struct ifaddrs *myaddrs;
          getifaddrs (&myaddrs);
          if (myaddrs->ifa_addr->ss_family == AF_INET) {
          }
        ], have_sockaddr_storage=yes, have_sockaddr_storage=no)
        if test "x$have_sockaddr_storage" = "xyes"; then
           AC_DEFINE(RFC2553, [], [Define to if follow RFC2553 ])
        fi
fi


dnl active plugins for org.gnome.Vinagre.gschema.xml
ACTIVE_PLUGINS="vnc"
if test "x$enable_ssh" = "xyes" ; then
ACTIVE_PLUGINS="$ACTIVE_PLUGINS,ssh"
fi
AC_SUBST(ACTIVE_PLUGINS)

VINAGRE_IMPLIB="-lvinagre-${VINAGRE_API_VERSION}"
PLUGIN_LIBTOOL_FLAGS="-module -avoid-version"

AC_SUBST(VINAGRE_IMPLIB)
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

VINAGRE_PLUGINS_DATA_DIR="$datadir/vinagre-1/plugins"
AC_SUBST(VINAGRE_PLUGINS_DATA_DIR)

VINAGRE_PLUGINS_LIBS_DIR="$libdir/vinagre-1/plugins"
AC_SUBST(VINAGRE_PLUGINS_LIBS_DIR)

# Introspection
GOBJECT_INTROSPECTION_CHECK([0.9.3])

if test "$found_introspection" = "yes"; then
	enable_introspection=yes
	AC_DEFINE([ENABLE_INTROSPECTION], [1], [Define to enable GObject Introspection])
else
	enable_introspection=no
fi

#*******************************************************************************
AC_CONFIG_FILES([
Makefile
data/Makefile
data/vinagre.desktop.in
data/vinagre-file.desktop.in
data/vinagre.pc
data/icons/Makefile
help/Makefile
po/Makefile.in
plugins/Makefile
plugins/vnc/Makefile
plugins/ssh/Makefile
plugins/rdp/Makefile
plugins/spice/Makefile
plugins/reverse-vnc/Makefile
plugins/im-status/Makefile
vinagre/Makefile
vinagre/view/Makefile
])
AC_OUTPUT

#*******************************************************************************

echo "

This is vinagre version AC_PACKAGE_VERSION

Configure summary:

	Compiler ...................:  ${CC}
	Compiler Flags .............:  ${CFLAGS}
	GObject Introspection.......:  $enable_introspection

	Avahi enabled ..............:  ${enable_avahi}
	GNOME Panel applet..........:  ${enable_applet}
	Telepathy support ..........:  ${have_telepathy}
	SSH plugin .................:  ${enable_ssh}
	RDP plugin .................:  ${enable_rdp}
	Spice plugin ...............:  ${enable_spice}
"
