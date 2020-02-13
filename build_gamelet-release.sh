#!/bin/bash

if [ ! -d build-release ]; then
  mkdir build-release
fi

cd build-release
if [ ! -f toolchain.cmake ]; then
  cp ../contrib/toolchains/toolchain-linux-default-release.cmake toolchain.cmake
fi

cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake .. || exit 4
cmake --build . -j 8
