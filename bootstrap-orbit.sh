#!/bin/bash

# Build vcpkg
cd external/vcpkg

if [ -f "vcpkg" ]
then
    echo "Orbit: found vcpkg"
else
    git submodule init
    git submodule update
    echo "Orbit: compiling vcpkg"
    ./bootstrap-vcpkg.sh
fi

## Build dependencies
set VCPKG_DEFAULT_TRIPLET=x64-linux
./vcpkg install freetype-gl curl breakpad capstone asio cereal imgui #freeglut glew

# CMake
cd ../..
if [ ! -d build/ ]; then
mkdir build
fi
cd build
cmake ..
cd ..
