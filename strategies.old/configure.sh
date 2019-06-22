#!/bin/bash
rm -rf build/debug
mkdir -p build/debug
cd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MODULE_PATH=$HOME/dev/amd64/share/cmake/Modules ../..
