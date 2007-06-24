## for automake to generate Makefile.in
bin_PROGRAMS = palx
palx_SOURCES = src/palx.cpp src/UI.cpp src/fade.cpp src/keyboard.cpp src/playrng.cpp src/script.cpp src/audio.cpp src/game.cpp src/mainloop.cpp src/qbrand.cpp src/team.cpp src/battle.cpp src/graph.cpp src/message.cpp src/resource.cpp src/timing.cpp src/begin.cpp src/internal.cpp src/palx.cpp src/scene.cpp PalLibrary/src/rle.cpp PalLibrary/src/yj1.cpp PalLibrary/src/rng.cpp
palx_LIBS = -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lole32 -ldinput -lddraw -ldxguid -lwinmm -ldsound -lalfont -lbinio -ladplug -lalleg_s
INCLUDES= -I PalLibrary/include
CFLAGS = -DALLEGRO_STATICLINK -mpentium -O2 -ffast-math -fomit-frame-pointer