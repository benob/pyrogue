#!/bin/bash

if [ $# != 2 ]; then
	echo "usage: $0 <game-zip> <output-dir>" >&2
	exit 1
fi

set -e -u -o pipefail

game_zip=$1
output_dir=$2

name=`basename "$game_zip" .zip`

mkdir -p "$output_dir"
dir=`dirname "$0"`
cp "$dir"/../../build-js/pyrogue.{js,wasm} "$output_dir"
cp "$game_zip" "$output_dir"

size=$(stat --printf="%s" "$game_zip")
sed 's/@@game_name@@/'$name'/g;s/@@game_size@@/'$size'/g' "$dir/pyrogue.preload.js.in" > "$output_dir"/pyrogue.preload.$name.js
sed 's/@@game_name@@/'$name'/g' "$dir/index.html.in" > "$output_dir/$name.html"
echo "run a web server pointed to '$output_dir' and open $name.html"
