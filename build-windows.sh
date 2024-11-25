#!/bin/sh -xe

CC="${CC:-clang}"
CFLAGS="-std=c99 -Wall -Wextra -Wdouble-promotion -pedantic -Wno-implicit-fallthrough -Wvla -Wl,--subsystem,windows"
LDLIBS="-lopengl32 -lgdi32 -lwinmm"

BUILD_FLAGS="-O3 -s -march=native"
DEBUG_FLAGS="-Og -ggdb"

RAYLIB_PATH="raylib-5.0_win64_mingw-w64"

$CC $CFLAGS $BUILD_FLAGS -o src/sound/wav2h.exe src/sound/wav2h.c -I$RAYLIB_PATH/include $RAYLIB_PATH/lib/libraylib.a $LDLIBS

./src/sound/wav2h.exe src/sound/*.wav

$CC $CFLAGS $BUILD_FLAGS -o game.exe src/*.c -I$RAYLIB_PATH/include $RAYLIB_PATH/lib/libraylib.a $LDLIBS
