## for automake to generate Makefile.in
bin_PROGRAMS = palx
palx_SOURCES = UI.cpp      fade.cpp      keyboard.cpp  playrng.cpp   script.cpp audio.cpp   game.cpp      mainloop.cpp  qbrand.cpp    team.cpp battle.cpp  graph.cpp     message.cpp   resource.cpp  timing.cpp begin.cpp   internal.cpp  palx.cpp      scene.cpp  celestial.cpp ../PalLibrary/src/yj1.cpp ../PalLibrary/src/rle.cpp ../PalLibrary/src/rng.cpp
#palx_LDADD = -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lole32 -ldinput -lddraw -ldxguid -lwinmm -ldsound -lalfont -lbinio -ladplug -lalleg_s
palx_LDADD = -lalfont -lalleg -ladplug -lbinio
INCLUDES= -I$(top_srcdir)/PalLibrary/include
#-DALLEGRO_STATICLINK 
AM_CPPFLAGS = -mtune=pentium -O2 -ffast-math -fomit-frame-pointer

if HAVE_WINDRES
palx_LDADD += palx_ico.o
endif

EXTRA_DIST = icon.rc celestial.ico
palx_ico.o: icon.rc celestial.ico
	$(WINDRES) icon.rc palx_ico.o