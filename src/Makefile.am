AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"\
	-I/usr/include/freetype2\
	-DENABLE_SPLASH=$(enable_splash)
	$(FTGL_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g


#datadir = @datadir@
#localedir = $(datadir)/locale
#DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = zaz

zaz_SOURCES = \
	main.cpp\
	common.h \
	error.h \
	audiobuffer.cpp \
	audiobuffer.h \
	mixer.cpp \
	mixer.h \
	oggsample.cpp \
	sample.h \
	scene.cpp \
	scene.h \
	settings.cpp \
	settings.h \
	wavesample.cpp \
	streamingoggsample.cpp \
	frame_events.cpp \
	frame_events.h \
	ogvexport.h \
	ogvexport.cpp \
	bezier.h \
	bezier.cpp \
	textureloader.h \
	textureloader.cpp \
	player.h \
	player.cpp \
	level.h \
	level.cpp \
	ballpath.h \
	ballpath.cpp \
	game.h \
	game.cpp \
	editor.h \
	editor.cpp \
	mainmenu.h \
	mainmenu.cpp \
	menu.h \
	menu.cpp \
	gameloop.h \
	gameloop.cpp \
	hiscores.h \
	hiscores.cpp \
	lineeditor.h \
	lineeditor.cpp \
	gettext.h \
	tests.h \
	tests.cpp \
	profile.h \
	profile.cpp \
	directorylister.cpp \
	directorylister.h \
	levelset.h \
	levelset.cpp \
	splash.h \
	splash.cpp 

zaz_CXXFLAGS = $(ZAZ_CFLAGS)

zaz_LDADD = $(ZAZ_LIBS) $(FTGL_LIBS) $(GL_LIBS) @LIBINTL@

