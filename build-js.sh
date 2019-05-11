#!/bin/sh

git submodule update --recursive

mkdir -p build-js
cd build-js
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
make -j 4
cp ../platform/js/* .
echo "run 'emrun index.html' to start pyrogue in a browser"
