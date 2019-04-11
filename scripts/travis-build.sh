#!/bin/sh

set -e
set -u

ROOT=$(pwd)

make
make clean

mkdir cmake-build
cd cmake-build
cmake ..
cmake --build .
cd ..

$HOME/.local/bin/meson . meson-build
cd meson-build
ninja
cd ..

${ROOT}/scripts/version.sh check
