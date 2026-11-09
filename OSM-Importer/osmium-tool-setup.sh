#!/bin/bash
cd "$(dirname "$0")"

echo "Downloading necessary packages"
git clone https://github.com/mapbox/protozero
git clone https://github.com/osmcode/libosmium
git clone https://github.com/osmcode/osmium-tool.git
echo "Building osmium-tool"
mkdir osmium-tool/build
cd osmium-tool/build
cmake ..
make
cd ../..

