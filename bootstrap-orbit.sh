#!/bin/bash

# Install required dependencies
sudo add-apt-repository universe
sudo apt-get update
sudo apt-get install -y build-essential cmake
sudo apt-get install -y libglu1-mesa-dev mesa-common-dev libxmu-dev libxi-dev 
sudo apt-get install -y linux-tools-common

# Build vcpkg
cd external/vcpkg

if [ -f "vcpkg" ]
then
    echo "Orbit: found vcpkg"
else
    git submodule update --init .
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
cmake -DCMAKE_BUILD_TYPE=Debug ..
#cmake ..
make
