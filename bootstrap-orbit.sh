#!/bin/bash

# Install required dependencies
sudo add-apt-repository universe
if [ $? -ne 0 ]; then
  sudo apt-get install -y software-properties-common
  sudo add-apt-repository universe
fi
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja
sudo apt-get install -y libglu1-mesa-dev mesa-common-dev libxmu-dev libxi-dev 
sudo apt-get install -y linux-tools-common

# Load Submodules (vcpkg, libunwindstack)
git submodule update --init --recursive

# Build vcpkg
cd external/vcpkg

if [ -f "vcpkg" ]
then
    echo "Orbit: found vcpkg"
else
    echo "Orbit: compiling vcpkg"
    ./bootstrap-vcpkg.sh
fi

## Build dependencies
./vcpkg install freetype freetype-gl breakpad \
  capstone asio cereal imgui freeglut glew

# CMake
cd ../..
if [ ! -d build/ ]; then
  mkdir build
fi

cd build
if [ ! -f toolchain.cmake ]; then
  cp ../contrib/toolchains/toolchain-linux-default-release.cmake toolchain.cmake
fi
cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -G Ninja ..
cmake --build .
