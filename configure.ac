AC_PREREQ([2.64])
AC_INIT([Vinagre],[2.91.92],[https://bugzilla.gnome.org/enter_bug.cgi?product=vinagre],[vinagre],[http://projects.gnome.org/vinagre/])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([vinagre/vinagre-main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.10 -Wall check-news no-define nostdinc subdir-objects])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

GNOME_COMPILE_WARNINGS([maximum])
GNOME_MAINTAINER_MODE_DEFINES

LT_PREREQ([2.2.6])
LT_INIT([disable-static])

PKG_PROG_PKG_CONFIG([0.16])

AC_PROG_SED

# TODO: Bump before release with GTK+ 3 support.
# TODO: Use in plugin path and where appropriate.
AC_SUBST([VINAGRE_ABI_VERSION], [1.0])

AC_SUBST([PLUGIN_LIBTOOL_FLAGS], ["-module -avoid-version"])
AC_SUBST([VINAGRE_PLUGINS_DATA_DIR], ["$datadir/vinagre-1/plugins"])
AC_SUBST([VINAGRE_PLUGINS_LIBS_DIR], ["$libdir/vinagre-1/plugins"])

GLIB_DEPS="glib-2.0 >= 2.25.11 gio-unix-2.0 >= 2.25.11"
GTHREAD_DEPS="gthread-2.0 >= 2.0.0"
GTK_DEPS="gtk+-3.0 >= 2.99.3"
GTK_VNC_DEPS="gtk-vnc-2.0 >= 0.4.3"
XML2_DEPS="libxml-2.0 >= 2.6.31"
PEAS_DEPS="libpeas-1.0 >= 0.7.2 libpeas-gtk-1.0 >= 0.7.2"

# Whether to enable support for SSH.
SSH_DEPS="vte-2.90 >= 0.20 $XML2_DEPS"

AC_ARG_ENABLE([ssh],
  [AS_HELP_STRING([--disable-ssh],
    [Disable Secure Shell (SSH) support])])

AS_IF([test "x$enable_ssh" != "xno"],
  [PKG_CHECK_EXISTS([$SSH_DEPS],
    [have_ssh=yes],
    [have_ssh=no])],
  [have_ssh=no])

AS_IF([test "x$have_ssh" = "xyes"],
  [AC_DEFINE([VINAGRE_ENABLE_SSH], [], [Build with SSH support])
    AC_PATH_PROG([SSH_PROGRAM], [ssh], [ssh])],
  [SSH_DEPS=""
    AS_IF([test "x$enable_ssh" = "xyes"],
      [AC_MSG_ERROR([SSH support requested but required dependencies not found])])])

AM_CONDITIONAL([VINAGRE_ENABLE_SSH], [test "x$enable_ssh" = "xyes"])

# Whether to enable support for RDP.
RDP_DEPS="$XML2_DEPS"
AC_ARG_ENABLE([rdp],
  [AS_HELP_STRING([--disable-rdp],
    [Disable Remote Desktop Protocol (RDP) support])])

AS_IF([test "x$enable_rdp" != "xno"],
  [PKG_CHECK_EXISTS([$RDP_DEPS],
    [have_rdp=yes],
    [have_rdp=no])],
  [have_rdp=no])

AS_IF([test "x$have_rdp" = "xyes"],
  [AC_DEFINE([VINAGRE_ENABLE_RDP], [], [Build with RDP support])
    AC_PATH_PROG([RDESKTOP_PROGRAM], [rdesktop], [rdesktop])],
  [RDP_DEPS=""
    AS_IF([test "x$enable_rdp" = "xyes"],
      [AC_MSG_ERROR([RDP support requested by required dependencies not found])])])

AM_CONDITIONAL([VINAGRE_ENABLE_RDP], [test "x$enable_rdp" = "xyes"])

# Check for spice.
SPICE_DEPS="spice-client-gtk-3.0 >= 0.5"

AC_ARG_ENABLE([spice],
  [AS_HELP_STRING([--disable-spice],
    [Disable SPICE protocol support])])

AS_IF([test "x$enable_spice" != "xno"],
  [PKG_CHECK_MODULES([SPICE],
    [$SPICE_DEPS],
    [have_spice=yes],
    [have_spice=no])],
  [have_spice=no])

AS_IF([test "x$have_spice" = "xyes"],
  [AC_DEFINE([VINAGRE_ENABLE_SPICE], [], [Build with spice support])],
  [SPICE_DEPS=""
    AS_IF([test "x$enable_spice" = "xyes"],
    [AC_MSG_ERROR([spice support requested but not found])])])

AM_CONDITIONAL([VINAGRE_ENABLE_SPICE], [test "x$have_spice" = "xyes"])

# Checks for header files. See vinagre/pyt_open.c for usage.
AC_CHECK_HEADERS([ifaddrs.h sys/un.h stropts.h termios.h utmp.h])

# Internationalization.
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])
AC_SUBST([GETTEXT_PACKAGE], [$PACKAGE_TARNAME])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [GETTEXT package name])
IT_PROG_INTLTOOL([0.40.0])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T

# Checks for library functions. See vinagre/pty_open.c for usage.
AC_CHECK_FUNCS([getpt posix_openpt grantpt unlockpt ptsname ptsname_r])

AC_SEARCH_LIBS([strerror], [cposix])

# Check for Avahi.
AVAHI_DEPS="avahi-ui-gtk3 >= 0.6.26 avahi-gobject >= 0.6.26"

AC_ARG_WITH([avahi],
  [AS_HELP_STRING([--without-avahi],
    [ignore avahi support and disable it])])

AS_IF([test "x$with_avahi" != "xno"],
  [PKG_CHECK_EXISTS([$AVAHI_DEPS],
    [have_avahi=yes],
    [have_avahi=no])],
  [have_avahi=no])

AS_IF([test "x$have_avahi" = "xyes"],
  [AC_DEFINE([VINAGRE_HAVE_AVAHI], [], [Build with avahi support])],
  [AVAHI_DEPS=""
    AS_IF([test "x$with_avahi" = "xyes"],
    [AC_MSG_ERROR([avahi support requested but not found])])])

AM_CONDITIONAL([VINAGRE_HAVE_AVAHI], [test "x$have_avahi" = "xyes"])

# Check for telepathy-glib.
TELEPATHY_GLIB_DEPS="telepathy-glib >= 0.11.6"

AC_ARG_WITH([telepathy],
  [AS_HELP_STRING([--without-telepathy],
    [ignore the presence of telepathy and disable it])])

AS_IF([test "x$with_telepathy" != "xno"],
  [PKG_CHECK_EXISTS([$TELEPATHY_GLIB_DEPS],
    [have_telepathy=yes],
    [have_telepathy=no])],
  [have_telepathy=no])

AS_IF([test "x$have_telepathy" = "xyes"],
  [AC_DEFINE([VINAGRE_HAVE_TELEPATHY_GLIB], [], [Build with telepathy-glib support])],
  [TELEPATHY_GLIB_DEPS=""
    AS_IF([test "x$with_telepathy" = "xyes"],
    [AC_MSG_ERROR([telepathy support requested but not found])])])

AM_CONDITIONAL([VINAGRE_HAVE_TELEPATHY_GLIB], [test "x$have_telepathy" = "xyes"])

# Check for panelapplet.
# TODO: Applet needs porting to GTK+ 3 and libpanelapplet-4.0, and is currently
# broken. Do not enable applet support unless you are a developer who wants to
# hack on it.
PANELAPPLET_DEPS="libpanelapplet-4.0 >= 2.91.91"

AC_ARG_WITH([panelapplet],
  [AS_HELP_STRING([--without-panelapplet],
    [ignore libpanelapplet support and disable it])])

AS_IF([test "x$with_panelapplet" != "xno"],
  [PKG_CHECK_MODULES([PANELAPPLET],
    [$PANELAPPLET_DEPS],
    [have_panelapplet=yes],
    [have_panelapplet=no])],
  [have_panelapplet=no])

AS_IF([test "x$have_panelapplet" = "xyes"],
  [AC_MSG_CHECKING([for panel applet path in pkg-config file])
    AC_SUBST([LIBPANEL_APPLET_DIR], [`$PKG_CONFIG --variable=libpanel_applet_dir libpanelapplet-4.0`])
    AC_MSG_RESULT([$LIBPANEL_APPLET_DIR])],
  [PANELAPPLET_DEPS=""
    AS_IF([test "x$with_panelapplet" = "xyes"],
      [AC_MSG_ERROR([panelapplet support requested but not found])])])

AM_CONDITIONAL([VINAGRE_HAVE_PANELAPPLET], [test "x$have_panelapplet" = "xyes"])

# Check required libraries
PKG_CHECK_MODULES([VINAGRE], [$GLIB_DEPS $GTHREAD_DEPS $GTK_DEPS gnome-keyring-1 $XML2_DEPS $PEAS_DEPS $AVAHI_DEPS $TELEPATHY_GLIB_DEPS])

PKG_CHECK_MODULES([VNC], [$GTK_VNC_DEPS $GTK_DEPS $XML2_DEPS])

# *** Checks for pty stuff ***
# Pull in the right libraries for various functions which might not be
# bundled into an exploded libc.
AC_CHECK_FUNC([socketpair],
  [have_socketpair=yes],
  [AC_CHECK_LIB([socket],
    [socketpair],
    [have_socketpair=yes
      LIBS="$LIBS -lsocket"])])

AS_IF([test "x$have_socketpair" = "xyes"],
  [AC_DEFINE([VINAGRE_HAVE_SOCKETPAIR], [], [Define if you have the socketpair function.])])

AC_SEARCH_LIBS([login_tty],
  [util],
  [AC_DEFINE([HAVE_LOGIN_TTY], [], [Whether login_tty is available])])

# Check for bind in libsocket (needed on Solaris)
AC_CHECK_LIB([socket], [bind])

# Check for getifaddrs
AM_CONDITIONAL([VINAGRE_HAVE_SELF_IFADDRS], [test "x$ac_cv_header_ifaddrs_h" != "xyes"])

# RFC2553 introduce sockaddr_storage as ifa_addr member in ifaddrs structure
# Not all distros follow this.
AS_IF([test "x$ac_cv_header_ifaddrs_h" = "xyes"],
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <net/if.h>
    #include <ifaddrs.h>
    ]], [[
    struct ifaddrs *myaddrs;
    getifaddrs (&myaddrs);
    if (myaddrs->ifa_addr->ss_family == AF_INET) {
    }
    ]])],
    [have_sockaddr_storage=yes],
    [have_sockaddr_storage=no])],
  [AS_IF([test "x$have_sockaddr_storage" = "xyes"],
    [AC_DEFINE([RFC2553], [], [Define if RFC2553 is followed])])])

# Active plugins for org.gnome.Vinagre.gschema.xml
# TODO: Make this work.
ACTIVE_PLUGINS="vnc"
AS_IF([test "x$enable_ssh" = "xyes"],
  [ACTIVE_PLUGINS="$ACTIVE_PLUGINS,ssh"])
AC_SUBST([ACTIVE_PLUGINS])

# Introspection
GOBJECT_INTROSPECTION_CHECK([0.9.3])

AS_IF([test "x$found_introspection" = "xyes"],
  [enable_introspection=yes
    AC_DEFINE([VINAGRE_HAVE_INTROSPECTION], [], [Build with GObject Introspection support])],
  [enable_introspection=no])

AM_CONDITIONAL([VINAGRE_HAVE_INTROSPECTION], [test "x$enable_introspection" = "xyes"])

GNOME_DOC_INIT

GLIB_GSETTINGS

AC_CONFIG_FILES([
Makefile
data/org.gnome.panel.Vinagre.panel-applet.in
data/vinagre.desktop.in
data/vinagre-file.desktop.in
data/vinagre-${VINAGRE_ABI_VERSION}.pc:data/vinagre.pc.in
help/Makefile
po/Makefile.in
])
AC_OUTPUT
