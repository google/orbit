#!/bin/bash

cd build
cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -G Ninja .. || exit 4
cmake --build . -j 16