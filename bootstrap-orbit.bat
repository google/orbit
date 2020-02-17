@echo off

:: Possible values are 141 for Visual Studio 2017 or 142 for Visual Studio 2019
set ORBIT_VS_VERSION=141

:: Build vcpkg
call git submodule update --init
cd external/vcpkg

if exist "vcpkg.exe" (
    echo found vcpkg.exe
) else (
    if %ORBIT_VS_VERSION% EQU 141 (
        echo set^(VCPKG_PLATFORM_TOOLSET v141^) >> triplets\x86-windows.cmake
        echo set^(VCPKG_PLATFORM_TOOLSET v141^) >> triplets\x64-windows.cmake
    ) else (
        echo set^(VCPKG_PLATFORM_TOOLSET v142^) >> triplets\x86-windows.cmake
        echo set^(VCPKG_PLATFORM_TOOLSET v142^) >> triplets\x64-windows.cmake
    )
    call ./bootstrap-vcpkg.bat
)

:: 32 bit
set VCPKG_DEFAULT_TRIPLET=x86-windows
vcpkg install abseil freeglut glew freetype freetype-gl curl breakpad capstone asio cereal imgui qt5-base gtest

:: Build dynamic dependencies
set VCPKG_DEFAULT_TRIPLET=x64-windows
vcpkg install abseil freeglut glew freetype freetype-gl curl breakpad capstone asio cereal imgui qt5-base gtest

cd ..\..

:: Fix breakpad missing file
copy "external\vcpkg\buildtrees\breakpad\src\f427f61ed3-fe83a49e5d\src\processor\linked_ptr.h" "external\vcpkg\installed\x86-windows\include\google_breakpad\processor\linked_ptr.h" /y
copy "external\vcpkg\buildtrees\breakpad\src\f427f61ed3-fe83a49e5d\src\processor\linked_ptr.h" "external\vcpkg\installed\x64-windows\include\google_breakpad\processor\linked_ptr.h" /y

:: CMake build
mkdir build_release_x86
copy "contrib\toolchains\toolchain-windows-32bit-msvc-release.cmake" "build_release_x86\toolchain.cmake" /y
cd build_release_x86
if %ORBIT_VS_VERSION% EQU 141 (
    cmake -DCMAKE_TOOLCHAIN_FILE="toolchain.cmake" -G "Visual Studio 15 2017 Win32" ..
) else (
    cmake -DCMAKE_TOOLCHAIN_FILE="toolchain.cmake" -G "Visual Studio 16 2019" -A Win32 ..
)
cmake --build . --target ALL_BUILD --config Release
cd ..

mkdir build_release_x64
copy "contrib\toolchains\toolchain-windows-64bit-msvc-release.cmake" "build_release_x64\toolchain.cmake" /y
cd build_release_x64
if %ORBIT_VS_VERSION% EQU 141 (
    cmake -DCMAKE_TOOLCHAIN_FILE="toolchain.cmake" -G "Visual Studio 15 2017 Win64" ..
) else (
    cmake -DCMAKE_TOOLCHAIN_FILE="toolchain.cmake" -G "Visual Studio 16 2019" -A x64 ..
)
cmake --build . --target ALL_BUILD --config Release
cd ..
