#!/bin/bash

# Load Submodules (vcpkg, libunwindstack)
git submodule update --init --recursive

if [ $? -ne 0 ]; then
  echo "Orbit: Could not update/initialize all the submodules. Exiting..."
  exit 1
fi

# Build vcpkg
cd external/vcpkg

if [ -f "vcpkg" ]; then
  echo "Orbit: found vcpkg"
else
  echo "Orbit: compiling vcpkg"
  ./bootstrap-vcpkg.sh
  if [ $? -ne 0 ]; then
    echo "Orbit: Could not bootstrap vcpkg. Exiting..."
    exit 2
  fi
fi

## Build dependencies
./vcpkg --overlay-triplets=../../contrib/vcpkg/triplets/ \
  --triplet x64-linux-ggp install abseil freetype freetype-gl breakpad \
  capstone asio cereal imgui freeglut glew curl gtest

if [ $? -ne 0 ]; then
  echo -n "Orbit: Could not install all the dependencies. "
  echo "Check for vcpkg error messages. Exiting..."
  exit 3
fi

cd ../..

## Build
if [ ! -d build_ggp_release ]; then
  mkdir build_ggp_release
fi

cd build_ggp_release/

if [ ! -f toolchain.cmake ]; then
  cp ../contrib/toolchains/toolchain-linux-ggp-release.cmake toolchain.cmake
fi

cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -G Ninja ..
ninja