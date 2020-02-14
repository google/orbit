#!/bin/bash

# Install required dependencies
sudo add-apt-repository universe
if [ $? -ne 0 ]; then
  sudo apt-get install -y software-properties-common
  sudo add-apt-repository universe
fi
sudo apt-get update
sudo apt-get install -y build-essential ninja-build bison flex
sudo apt-get install -y libglu1-mesa-dev mesa-common-dev libxmu-dev libxi-dev
sudo apt-get install -y linux-tools-common qt5-default

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
./vcpkg install abseil freetype freetype-gl breakpad \
  capstone asio cereal imgui freeglut glew curl gtest

if [ $? -ne 0 ]; then
  echo -n "Orbit: Could not install all the dependencies. "
  echo "Check for vcpkg error messages. Exiting..."
  exit 3
fi

cd ../..

# Build
if [[ $(uname -a) == *"yeti"* ]]; then
  ./build_gamelet.sh
else
  ./build.sh
fi
