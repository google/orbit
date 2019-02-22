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

## Override freetype-gl portfile for linux (.lib->.a)
cd ../..
cp "OrbitUtils/freetype-gl-portfile.cmake" "external/vcpkg/ports/freetype-gl/portfile.cmake"
cd external/vcpkg

## Build dependencies
set VCPKG_DEFAULT_TRIPLET=x64-linux
./vcpkg install freetype-gl breakpad capstone asio cereal imgui #freeglut glew curl

#remove compiled libfreetype.a TODO: fix in cmakelists.txt
rm "installed/x64-linux/lib/libfreetype.a"

# CMake
cd ../..
if [ ! -d build/ ]; then
mkdir build
fi
cd build
cmake ..
make
