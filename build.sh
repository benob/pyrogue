#!/bin/sh

git submodule update --recursive

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j 4

