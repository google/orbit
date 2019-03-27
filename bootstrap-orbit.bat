@echo off

:: Check for QTDIR environment variable
if defined QTDIR (
    echo QTDIR=%QTDIR% 
) else ( 
    echo ======= ERROR =======
    echo QTDIR environment variable not found.
    echo Please set your QTDIR environment variable to point to your Qt installation directory [ex. C:\Qt\5.12.1\msvc2017_64]
    echo ======= ERROR =======
    goto :eof
)

:: Build vcpkg
call git submodule init
call git submodule update
cd external/vcpkg

if exist "vcpkg.exe" (
    echo found vcpkg.exe
) else (
    call ./bootstrap-vcpkg.bat
)

:: 32 bit
set VCPKG_DEFAULT_TRIPLET=x86-windows
vcpkg install asio cereal breakpad curl

:: Build dynamic dependencies
set VCPKG_DEFAULT_TRIPLET=x64-windows
vcpkg install freeglut glew freetype-gl curl breakpad capstone asio cereal imgui

:: Build static dependencies
set VCPKG_DEFAULT_TRIPLET=x64-windows-static
vcpkg install capstone freeglut imgui

cd ../..

:: Fix breakpad missing file
copy "external\vcpkg\buildtrees\breakpad\src\9e12edba6d-12269dd01c\src\processor\linked_ptr.h" "external\vcpkg\installed\x64-windows\include\google_breakpad\processor\linked_ptr.h" /y

:: CMake build/x64
mkdir build
cd build
mkdir x64
cd x64
cmake -DCMAKE_TOOLCHAIN_FILE='../../external/vcpkg/scripts/buildsystems/vcpkg.cmake' -DCMAKE_GENERATOR_PLATFORM=x64 ../..
cd ..

:: CMake build/x86
mkdir x86
cd x86
cmake -DCMAKE_TOOLCHAIN_FILE='../../external/vcpkg/scripts/buildsystems/vcpkg.cmake' ../..
cd ../..
