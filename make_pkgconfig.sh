#!/bin/bash

prefix=$1
output=$2

cat << pkgconfig > $output
prefix=$prefix
exec_prefix=\${prefix}
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: Wad64
Description: Simple library for working with Wad64 files
Version: 1.0.0
Libs: -L\${libdir} -lwad64
Cflags: -I\${includedir}
pkgconfig
