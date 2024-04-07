#!/bin/sh -xe

CC="${CC:-g++}"
CFLAGS="-std=c++11 -Wall -Wextra -Wdouble-promotion -pedantic -Wl,--subsystem,windows"
LDLIBS="-lopengl32 -lgdi32 -lwinmm"

BUILD_FLAGS="-O3 -march=native -mtune=native -s"
DEBUG_FLAGS="-Og -ggdb"

RAYLIB_PATH="raylib-5.0_win64_mingw-w64"

$CC $CFLAG $LDLIBS $BUILD_FLAGS -o src/sound/wav2h.exe src/sound/wav2h.cpp -I$RAYLIB_PATH/include $RAYLIB_PATH/lib/libraylib.a

./src/sound/wav2h.exe src/sound/*.wav

$CC $CFLAG $LDLIBS $BUILD_FLAGS -o game.exe src/*.cpp -I$RAYLIB_PATH/include $RAYLIB_PATH/lib/libraylib.a 
