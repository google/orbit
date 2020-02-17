@echo off

:: Build vcpkg
call git submodule update --init
cd external/vcpkg

if exist "vcpkg.exe" (
    echo found vcpkg.exe
) else (
    echo set^(VCPKG_PLATFORM_TOOLSET v142^) >> triplets\x64-windows.cmake
    call ./bootstrap-vcpkg.bat
)

:: Build dynamic dependencies
set VCPKG_DEFAULT_TRIPLET=x64-windows
vcpkg install abseil freeglut glew freetype freetype-gl curl breakpad capstone asio cereal imgui qt5-base gtest

cd ..\..

:: Fix breakpad missing file
copy "external\vcpkg\buildtrees\breakpad\src\f427f61ed3-fe83a49e5d\src\processor\linked_ptr.h" "external\vcpkg\installed\x86-windows\include\google_breakpad\processor\linked_ptr.h" /y
copy "external\vcpkg\buildtrees\breakpad\src\f427f61ed3-fe83a49e5d\src\processor\linked_ptr.h" "external\vcpkg\installed\x64-windows\include\google_breakpad\processor\linked_ptr.h" /y

mkdir build_release_x64
copy "contrib\toolchains\toolchain-windows-64bit-msvc-release.cmake" "build_release_x64\toolchain.cmake" /y
cd build_release_x64
cmake -DCMAKE_TOOLCHAIN_FILE="toolchain.cmake" -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --target ALL_BUILD --config Release
cd ..
