dnl Process this file with autoconf to produce a configure script.

AC_INIT(vinagre, 0.1)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

dnl make sure we keep ACLOCAL_FLAGS around for maintainer builds to work
AC_SUBST(ACLOCAL_AMFLAGS, "\${ACLOCAL_FLAGS}")

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC


dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
GETTEXT_PACKAGE=vinagre
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

AM_PROG_LIBTOOL
IT_PROG_INTLTOOL([0.35.0])
AM_GLIB_GNU_GETTEXT

CFLAGS="${CFLAGS} -Wall"

AC_ARG_ENABLE(deprecations,
              [AC_HELP_STRING([--disable-deprecations],
                              [don't warn about deprecated usages [default=no]])],,
              [enable_deprecations=yes])

if test "x$enable_deprecations" = "xyes"; then
   DISABLE_DEPRECATED_CFLAGS="\
-DG_DISABLE_DEPRECATED \
-DGDK_DISABLE_DEPRECATED \
-DGTK_DISABLE_DEPRECATED \
-DGDK_PIXBUF_DISABLE_DEPRECATED \
-DGNOME_DISABLE_DEPRECATED"
   AC_SUBST(DISABLE_DEPRECATED_CFLAGS)
fi

PKG_CHECK_MODULES(VINAGRE, [gtk+-2.0 >= 2.8 libglade-2.0 >= 2.6.0 gconf-2.0 ])
AC_SUBST(VINAGRE_CFLAGS)
AC_SUBST(VINAGRE_LIBS)

PKG_CHECK_MODULES(GTKVNC, [gnutls])
AC_SUBST(GTKVNC_CFLAGS)
AC_SUBST(GTKVNC_LIBS)

#
# Check for Avahi
#
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
		      [avahi-client >= 0.6.17  avahi-glib >= 0.6.17 ])
    AVAHI_CFLAGS="$AVAHI_CFLAGS -DVINAGRE_HAVE_AVAHI"
fi
AC_SUBST(AVAHI_CFLAGS)
AC_SUBST(AVAHI_LIBS)
AM_CONDITIONAL(VINAGRE_AVAHI, [test "x$VINAGRE_ENABLE_AVAHI" = "xyes"])

AC_OUTPUT([
Makefile
data/Makefile
data/vinagre.desktop.in
src/Makefile
src/gtk-vnc/Makefile
po/Makefile.in
])
