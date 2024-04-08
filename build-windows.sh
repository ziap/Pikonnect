#!/bin/sh -xe

CC="${CC:-g++}"
CFLAGS="-std=c++11 -Wall -Wextra -Wdouble-promotion -pedantic -Wno-implicit-fallthrough -Wl,--subsystem,windows"
LDLIBS="-lopengl32 -lgdi32 -lwinmm"

BUILD_FLAGS="-O3 -s"
DEBUG_FLAGS="-Og -ggdb"

RAYLIB_PATH="raylib-5.0_win64_mingw-w64"

$CC $CFLAGS $BUILD_FLAGS -o src/sound/wav2h.exe src/sound/wav2h.cpp -I$RAYLIB_PATH/include $RAYLIB_PATH/lib/libraylib.a $LDLIBS

./src/sound/wav2h.exe src/sound/*.wav

$CC $CFLAGS $BUILD_FLAGS -o game.exe src/*.cpp -I$RAYLIB_PATH/include $RAYLIB_PATH/lib/libraylib.a $LDLIBS
