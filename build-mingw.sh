#!/bin/sh

git submodule update --recursive

mkdir -p build-mingw
cd build-mingw
export PKG_CONFIG_PATH=/usr/x86_64-w64-mingw32/lib/pkgconfig/
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-toolchain.cmake 
make -j 4
cp ../platform/windows/*.dll .
