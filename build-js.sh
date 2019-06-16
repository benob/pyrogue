#!/bin/sh

git submodule update --recursive

cd micropython
git checkout py-gc-root-stack-v3
cd ..
mkdir -p build-js
cd build-js
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
make -j 4

mkdir dist
for example in fire.zip plasma.zip blob.zip game.zip tileset.zip; do
../platform/js/export-game.sh ../examples/zip/$example dist/
done
