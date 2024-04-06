#!/bin/sh -xe

CC="${CC:-g++}"
CFLAGS="-std=c++11 -Wall -Wextra -Wdouble-promotion -pedantic"

BUILD_FLAGS="-O3 -march=native -mtune=native -s"
DEBUG_FLAGS="-Og -ggdb"

RAYLIB_PATH="raylib-5.0_linux_amd64"

$CC $CFLAG $LDLIBS $BUILD_FLAGS -o game src/*.cpp -I$RAYLIB_PATH/include $RAYLIB_PATH/lib/libraylib.a
