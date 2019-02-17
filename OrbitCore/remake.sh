rm -rf build
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE='../../external/vcpkg/scripts/buildsystems/vcpkg.cmake' ..
cd build
make
