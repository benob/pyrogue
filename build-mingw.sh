#!/bin/sh
mkdir -p build-mingw
cd build-mingw
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-toolchain.cmake -DSDL2_INCLUDE_DIR:PATH=/usr/x86_64-w64-mingw32/include/SDL2
make -j 4
cp /usr/x86_64-w64-mingw32/bin/SDL2.dll .
