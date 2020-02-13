#!/bin/bash

if [ ! -d build-debug ]; then
  mkdir build-debug
fi

cd build-debug
if [ ! -f toolchain.cmake ]; then
  cp ../contrib/toolchains/toolchain-linux-default-debug.cmake toolchain.cmake
fi

cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -G Ninja .. || exit 4
cmake --build .
