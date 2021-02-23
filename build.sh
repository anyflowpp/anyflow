#!/bin/sh
DIR="$( cd "$(dirname "$0")" && pwd)"
cd build
cmake --build . --config Release