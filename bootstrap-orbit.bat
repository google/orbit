@echo off

:: Check for QTDIR environment variable
if defined Qt5_DIR (
    echo Qt5_DIR=%Qt5_DIR% 
) else ( 
    echo ======= ERROR =======
    echo Qt5_DIR environment variable not found.
    echo Please set your Qt5_DIR environment variable to point to your Qt installation directory [ex. C:\Qt\5.12.1\msvc2017_64\lib\cmake\Qt5]
    echo ======= ERROR =======
    goto :eof
)

:: Build vcpkg
call git submodule update --init
cd external/vcpkg

if exist "vcpkg.exe" (
    echo found vcpkg.exe
) else (
    call ./bootstrap-vcpkg.bat
)

:: 32 bit
set VCPKG_DEFAULT_TRIPLET=x86-windows
vcpkg install freeglut glew freetype freetype-gl curl breakpad capstone asio cereal imgui

:: Build dynamic dependencies
set VCPKG_DEFAULT_TRIPLET=x64-windows
vcpkg install freeglut glew freetype freetype-gl curl breakpad capstone asio cereal imgui

cd ..\..

:: Fix breakpad missing file
copy "external\vcpkg\buildtrees\breakpad\src\9e12edba6d-12269dd01c\src\processor\linked_ptr.h" "external\vcpkg\installed\x64-windows\include\google_breakpad\processor\linked_ptr.h" /y
copy "external\vcpkg\buildtrees\breakpad\src\9e12edba6d-12269dd01c\src\processor\linked_ptr.h" "external\vcpkg\installed\x86-windows\include\google_breakpad\processor\linked_ptr.h" /y

:: CMake build
mkdir build_release_x86
copy "contrib\toolchains\toolchain-windows-32bit-msvc2017-release.cmake" "build_release_x86\toolchain.cmake" /y
cd build_release_x86
cmake -DCMAKE_TOOLCHAIN_FILE="toolchain.cmake" -G "Visual Studio 15 2017" ..
cd ..

mkdir build_release_x64
copy "contrib\toolchains\toolchain-windows-64bit-msvc2017-release.cmake" "build_release_x64\toolchain.cmake" /y
cd build_release_x64
cmake -DCMAKE_TOOLCHAIN_FILE="toolchain.cmake" -G "Visual Studio 15 2017 Win64" ..
cd ..
