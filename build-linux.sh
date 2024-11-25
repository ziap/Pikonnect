#!/bin/sh -xe

CC="${CC:-clang}"
CFLAGS="-std=c99 -Wall -Wextra -Wdouble-promotion -Wno-implicit-fallthrough -Wvla -pedantic"
LDLIBS="-lm"

BUILD_FLAGS="-O3 -s -march=native"
DEBUG_FLAGS="-Og -ggdb"

RAYLIB_PATH="raylib-5.0_linux_amd64"

$CC $CFLAGS $BUILD_FLAGS -o src/sound/wav2h src/sound/wav2h.c -I$RAYLIB_PATH/include $RAYLIB_PATH/lib/libraylib.a $LDLIBS

./src/sound/wav2h src/sound/*.wav

$CC $CFLAGS $BUILD_FLAGS -o game src/*.c -I$RAYLIB_PATH/include $RAYLIB_PATH/lib/libraylib.a $LDLIBS
