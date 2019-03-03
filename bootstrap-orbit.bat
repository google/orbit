::Qt is in C:\Qt\5.8\msvc2015_64\

:: Build vcpkg
git submodule init
git submodule update
cd external/vcpkg

if exist "vcpkg.exe" (
    echo found vcpkg.exe
) else (
    call .\bootstrap-vcpkg.bat
)

:: Build dynamic dependencies
set VCPKG_DEFAULT_TRIPLET=x64-windows
vcpkg install freeglut glew freetype-gl curl breakpad capstone asio cereal imgui
:: websocketpp

:: Build static dependencies
set VCPKG_DEFAULT_TRIPLET=x64-windows-static
vcpkg install capstone freeglut imgui

:: CMake
cd ../..
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE='..\external\vcpkg\scripts\buildsystems\vcpkg.cmake' -DCMAKE_GENERATOR_PLATFORM=x64 ..

cd ..

copy "external\vcpkg\buildtrees\breakpad\src\9e12edba6d-12269dd01c\src\processor\linked_ptr.h" "external\vcpkg\installed\x64-windows\include\google_breakpad\processor\linked_ptr.h"
