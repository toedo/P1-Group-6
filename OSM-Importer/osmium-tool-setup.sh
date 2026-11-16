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

echo "Building Protozero"
mkdir protozero/build
cd protozero/build
cmake ..
make
cd ../..

echo "Building Libosmium"
mkdir libosmium/build
cd libosmium/build
cmake ..
make
cd ../..

echo "Making files executable and Creating symlinks to usr/bin"
# sudo ln -s ./osmium-tool/build/osmium /usr/bin


