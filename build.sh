#!/bin/sh


SOURCE="$1.c"
OUTPUT=$1

INCLUDES=`pkg-config --cflags gtk4`
LIBS=`pkg-config --libs gtk4`
gcc -Wall $INCLUDES $SOURCE $LIBS -lm -o build/$OUTPUT
