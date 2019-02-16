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

## Build dynamic dependencies
set VCPKG_DEFAULT_TRIPLET=x64-linux
./vcpkg install freeglut glew freetype-gl curl breakpad capstone asio cereal imgui
# websocketpp

# Build static dependencies
set VCPKG_DEFAULT_TRIPLET=x64-linux-static
./vcpkg install capstone freeglut imgui

# CMake
cd ../..
if [ ! -d build/ ]; then
mkdir build
fi
cd build
cmake -DCMAKE_TOOLCHAIN_FILE='..\external\vcpkg\scripts\buildsystems\vcpkg.cmake' -DCMAKE_GENERATOR_PLATFORM=x64 ..

cd ..


# TODO
# investigate how to remove dependency on boost, it's ridiculously slow to build.
# use vcpkg for Qt