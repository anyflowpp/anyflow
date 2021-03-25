#!/bin/sh
DIR="$( cd "$(dirname "$0")" && pwd)"
cd build
export CFLAGS="$CFLAGS -g -O0"
make -j8