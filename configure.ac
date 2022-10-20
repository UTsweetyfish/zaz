AC_INIT(zaz, 1.0.1)
AM_INIT_AUTOMAKE([-Wall -Werror])
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE
AM_GNU_GETTEXT([external])
AC_ISC_POSIX
AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_RANLIB

PKG_CHECK_MODULES(ZAZ, [sdl vorbis vorbisfile theoraenc theoradec])
PKG_CHECK_MODULES(FTGL, ftgl >= 2.1.3)

AC_CHECK_LIB(SDL_image, IMG_Load, [], AC_MSG_ERROR(no SDL_image found))
AC_CHECK_LIB(vorbisenc, vorbis_encode_init, [], AC_MSG_ERROR(no vorbisenc found))
AC_CHECK_LIB(GL, main, [], AC_MSG_ERROR([OpenGL library not found.]))

AC_CHECK_HEADER(GL/gl.h, [], AC_MSG_ERROR(no GL/gl.h found))
dnl AC_CHECK_HEADER(SDL/SDL_image.h, [], AC_MSG_ERROR(no SDL/SDL_image.h found))

AC_ARG_WITH(icondir,
	AS_HELP_STRING([--with-icondir=DIR],[icon files [DATAROOTDIR/pixmaps]]),
	[ with_icondir=${withval} ], [ with_icondir=${datadir}/pixmaps ])
AC_SUBST([icondir], [${with_icondir}])

AC_ARG_WITH(applicationdir,
	AS_HELP_STRING([--with-applicationdir=DIR],[application files [DATAROOTDIR/applications]]),
	[ with_applicationdir=${withval} ], [ with_applicationdir=${datadir}/applications ])
AC_SUBST([applicationdir], [${with_applicationdir}])

ac_enable_splash=false
AC_ARG_ENABLE([splash],
    AS_HELP_STRING([--enable-splash], [enable splash screen]),
    [ac_enable_splash=true], [ac_enable_splash=false])

AC_SUBST([enable_splash], [${ac_enable_splash}])

AC_OUTPUT([ po/Makefile.in
Makefile
data/Makefile
src/Makefile
extra/Makefile
])
