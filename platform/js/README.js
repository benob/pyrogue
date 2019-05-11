Generate data loader for single zip file:

python /usr/lib/emscripten/tools/file_packager.py game.zip --preload ../examples/game.zip@game.zip --js-output=pyrogue.preload.js
