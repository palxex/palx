## for automake to generate Makefile.in
bin_PROGRAMS = palx
palx_SOURCES = UI.cpp      fade.cpp      keyboard.cpp  playrng.cpp   script.cpp audio.cpp   game.cpp      mainloop.cpp  qbrand.cpp    team.cpp battle.cpp  graph.cpp     message.cpp   resource.cpp  timing.cpp begin.cpp   internal.cpp  palx.cpp      scene.cpp item.cpp shop.cpp config.cpp celestial.cpp luabinding.cpp ../PalLibrary/src/yj1.cpp ../PalLibrary/src/yj2.cpp ../PalLibrary/src/rle.cpp ../PalLibrary/src/rng.cpp ../alfont/alfont.c
palx_LDADD = $(adplug_LIBS) $(allegro_LIBS) $(freetype2_LIBS)
#$(BOOST_REGEX_LIB)
palx_CFLAGS= -I$(top_srcdir)/PalLibrary/include -I$(top_srcdir)/alfont $(allegro_CFLAGS) $(binio_CFLAGS) $(adplug_CFLAGS) $(freetype2_CFLAGS)

if LUA_CONSOLE
palx_LDADD += $(lua_LIBS)
palx_CFLAGS += $(lua_CFLAGS)
endif

palx_CXXFLAGS = $(palx_CFLAGS)

if HAVE_WINDRES
palx_LDADD += palx_ico.o
palx_LDFLAGS = -mwindows
endif

EXTRA_DIST = icon.rc celestial.ico
palx_ico.o: icon.rc celestial.ico
	$(WINDRES) icon.rc palx_ico.o
