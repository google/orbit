#!/bin/bash

# Install required dependencies
sudo add-apt-repository universe
if [ $? -ne 0 ]; then
  sudo apt-get install -y software-properties-common
  sudo add-apt-repository universe
fi
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build bison flex
sudo apt-get install -y libglu1-mesa-dev mesa-common-dev libxmu-dev libxi-dev 
sudo apt-get install -y linux-tools-common

# Dev dependencies:
# - cmake >= 3.15
# - ninja (optional)
# - bison (for compiling qt5)
# - flex (for compiling qt5)

# Load Submodules (vcpkg, libunwindstack)
git submodule update --init --recursive

if [ $? -ne 0 ]; then
  echo "Orbit: Could not update/initialize all the submodules. Exiting..."
  exit 1
fi

# Build vcpkg
cd external/vcpkg

if [ -f "vcpkg" ]
then
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
./vcpkg install freetype freetype-gl breakpad \
  capstone asio cereal imgui freeglut glew curl gtest qt5-base gtest

if [ $? -ne 0 ]; then
  echo "Orbit: Could not install all the dependencies. Check for vcpkg error messages. Exiting..."
  exit 3
fi

# CMake
cd ../..
if [ ! -d build/ ]; then
  mkdir build
fi

cd build
if [ ! -f toolchain.cmake ]; then
  cp ../contrib/toolchains/toolchain-linux-default-release.cmake toolchain.cmake
fi

cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -G Ninja .. || exit 4
cmake --build .
