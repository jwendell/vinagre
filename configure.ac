# Process this file with autoconf to produce a configure script.

AC_INIT(vinagre, 0.4.91)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

dnl make sure we keep ACLOCAL_FLAGS around for maintainer builds to work
AC_SUBST(ACLOCAL_AMFLAGS, "\${ACLOCAL_FLAGS}")

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AM_PROG_LIBTOOL

GNOME_COMPILE_WARNINGS(yes)

#*******************************************************************************
# Internationalization
#*******************************************************************************
GETTEXT_PACKAGE=vinagre
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

IT_PROG_INTLTOOL([0.35.0])
AM_GLIB_GNU_GETTEXT

GNOME_DOC_INIT

#*******************************************************************************
# Check required GNOME libraries
#*******************************************************************************

GLIB_REQUIRED=2.15.3
GTK_REQUIRED=2.12.0
LIBGLADE_REQUIRED=2.6.0
GCONF_REQUIRED=2.16.0
GTK_VNC_REQUIRED=0.3

PKG_CHECK_MODULES(VINAGRE, \ 
  glib-2.0 >= $GLIB_REQUIRED \
  gio-2.0 >= $GLIB_REQUIRED \
  gtk+-2.0 >= $GTK_REQUIRED \ 
  libglade-2.0 >= $LIBGLADE_REQUIRED \
  gconf-2.0 >= $GCONF_REQUIRED \
  gnome-keyring-1 \
  gtk-vnc-1.0 >= $GTK_VNC_REQUIRED)
AC_SUBST(VINAGRE_CFLAGS)
AC_SUBST(VINAGRE_LIBS)

#*******************************************************************************
# Check for Avahi
#*******************************************************************************

AVAHI_REQUIRED=0.6.18

AC_ARG_ENABLE(avahi,
        AS_HELP_STRING([--enable-avahi],[Enable use of avahi]),
        [case "${enableval}" in
                yes) VINAGRE_ENABLE_AVAHI=yes ;;
                no)  VINAGRE_ENABLE_AVAHI=no ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-avahi) ;;
        esac],
        [VINAGRE_ENABLE_AVAHI=no])

AVAHI_CFLAGS=
AVAHI_LIBS=
if test "x$VINAGRE_ENABLE_AVAHI" = "xyes" ; then
    PKG_CHECK_MODULES(AVAHI,
		      [avahi-ui >= $AVAHI_REQUIRED])
    AVAHI_CFLAGS="$AVAHI_CFLAGS -DVINAGRE_HAVE_AVAHI"
fi
AC_SUBST(AVAHI_CFLAGS)
AC_SUBST(AVAHI_LIBS)
AM_CONDITIONAL(VINAGRE_AVAHI, [test "x$VINAGRE_ENABLE_AVAHI" = "xyes"])

#*******************************************************************************

AC_OUTPUT([
Makefile
data/Makefile
data/vinagre.desktop.in
data/vinagre-file.desktop.in
data/icons/Makefile
help/Makefile
po/Makefile.in
src/Makefile
])

#*******************************************************************************

echo "

This is vinagre version AC_PACKAGE_VERSION

Configure summary:

	Compiler ...................:  ${CC} 
	Compiler Flags .............:  ${CFLAGS}

	Avahi support ..............:  ${VINAGRE_ENABLE_AVAHI}
"
