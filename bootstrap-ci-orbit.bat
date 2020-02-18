@echo off

:: Possible values are 141 for Visual Studio 2017 or 142 for Visual Studio 2019
set ORBIT_VS_VERSION=142
:: We require a specific version of the Winsdk to avoid anything random being picked up.
:: This is appended to the toolchain files for vcpkg and Orbit itself below.
set ORBIT_WINSDK_VERSION="10.0.18362.0"

call git submodule update --init
:: Copy vcpkg stuff instead of building it.
robocopy C:\vcpkg\scripts external\vcpkg\scripts /s /e /njh /njs /ndl /nc /ns /NFL
robocopy C:\vcpkg\packages external\vcpkg\packages /s /e /njh /njs /ndl /nc /ns /NFL
robocopy C:\vcpkg\installed external\vcpkg\installed /s /e /njh /njs /ndl /nc /ns /NFL
robocopy C:\vcpkg\downloads external\vcpkg\downloads /s /e /njh /njs /ndl /nc /ns /NFL
mkdir external\vcpkg\buildtrees
robocopy C:\vcpkg\buildtrees\freetype-gl external\vcpkg\buildtrees\freetype-gl /s /e /njh /njs /ndl /nc /ns /NFL
robocopy C:\vcpkg\buildtrees\breakpad external\vcpkg\buildtrees\breakpad /s /e /njh /njs /ndl /nc /ns /NFL

:: Fix breakpad missing file
copy "external\vcpkg\buildtrees\breakpad\src\f427f61ed3-fe83a49e5d\src\processor\linked_ptr.h" "external\vcpkg\installed\x64-windows\include\google_breakpad\processor\linked_ptr.h" /y

mkdir build_release_x64
copy "contrib\toolchains\toolchain-windows-64bit-msvc-release.cmake" "build_release_x64\toolchain.cmake" /y
echo set^(CMAKE_SYSTEM_VERSION %ORBIT_WINSDK_VERSION%^) >> build_release_x64\toolchain.cmake
cd build_release_x64
cmake -DCMAKE_TOOLCHAIN_FILE="toolchain.cmake" -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --target ALL_BUILD --config Release
cd ..
