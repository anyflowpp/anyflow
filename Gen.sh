#!/bin/sh
DIR="$(cd "$(dirname "$0")" && pwd)"

if [ ! -e build ] ; then
    mkdir build
else
    rm -rf build
    mkdir build
fi

cd build
cmake ..