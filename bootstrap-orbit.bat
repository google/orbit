:: Build vcpkg
call git submodule init
call git submodule update
cd external/vcpkg

if exist "vcpkg.exe" (
    echo found vcpkg.exe
) else (
    call ./bootstrap-vcpkg.bat
)

:: Build dynamic dependencies
set VCPKG_DEFAULT_TRIPLET=x64-windows
vcpkg install freeglut glew freetype-gl curl breakpad capstone asio cereal imgui
:: websocketpp

:: Build static dependencies
set VCPKG_DEFAULT_TRIPLET=x64-windows-static
vcpkg install capstone freeglut imgui

if not exist "installed/x64-windows/include/google_breakpad/processor/linked_ptr.h" (
call copy "buildtrees/breakpad/src/9e12edba6d-12269dd01c/src/processor/linked_ptr.h" "installed/x64-windows/include/google_breakpad/processor/linked_ptr.h"
)

:: CMake
cd ../..

mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE='../external/vcpkg/scripts/buildsystems/vcpkg.cmake' -DCMAKE_GENERATOR_PLATFORM=x64 ..

cd ..
