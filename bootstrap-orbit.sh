#!/bin/bash

#install required dependencies
sudo add-apt-repository universe
sudo apt-get update
sudo apt-get install -y curl build-essential libcurl4-openssl-dev unzip cmake tar
sudo apt-get install -y libglu1-mesa-dev mesa-common-dev libxi-dev 
sudo apt-get install -y libfreetype6-dev freeglut3-dev qt5-default

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
./vcpkg install freetype-gl breakpad capstone asio cereal imgui glew

#remove compiled libfreetype.a TODO: fix in cmakelists.txt
rm "installed/x64-linux/lib/libfreetype.a"

# CMake
cd ../..
if [ ! -d build/ ]; then
mkdir build
fi
cd build
#cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake ..
make
cd OrbitQt
./OrbitQt
