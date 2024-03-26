#!/bin/sh -xe

CC="${CC:-g++}"
CFLAGS="-std=c++11 -Wall -Wextra -Wdouble-promotion -pedantic"

BUILD_FLAGS="-O3 -march=native -mtune=native -s"
DEBUG_FLAGS="-Og -ggdb"


$CC $CFLAG $LDLIBS $DEBUG_FLAGS -o game src/*.cpp -Iraylib-5.0_linux_amd64/include raylib-5.0_linux_amd64/lib/libraylib.a
