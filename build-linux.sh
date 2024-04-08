#!/bin/sh -xe

CC="${CC:-g++}"
CFLAGS="-std=c++11 -Wall -Wextra -Wdouble-promotion -Wno-implicit-fallthrough -pedantic"
LDLIBS=""

BUILD_FLAGS="-O3 -march=native -mtune=native -s"
DEBUG_FLAGS="-Og -ggdb"

RAYLIB_PATH="raylib-5.0_linux_amd64"

$CC $CFLAGS $LDLIBS $BUILD_FLAGS -o src/sound/wav2h src/sound/wav2h.cpp -I$RAYLIB_PATH/include $RAYLIB_PATH/lib/libraylib.a

./src/sound/wav2h src/sound/*.wav

$CC $CFLAGS $LDLIBS $BUILD_FLAGS -o game src/*.cpp -I$RAYLIB_PATH/include $RAYLIB_PATH/lib/libraylib.a
