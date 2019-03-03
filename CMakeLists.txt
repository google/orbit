cmake_minimum_required(VERSION 3.6)

# Note: (Windows + vcpkg) to build and use x64:
# a) set VCPKG_DEFAULT_TRIPLET=x64-windows, then install all required dependencies
# b) invoke cmake for 64 bit platform, as follows:
#    cmake -DCMAKE_GENERATOR_PLATFORM=x64 ..
# or, i.e.: 
#    cmake -G "Visual Studio 15 2017 Win64" ..
# This assumes CMAKE_TOOLCHAIN_FILE is correctly set, in accordance with
# https://github.com/Microsoft/vcpkg/blob/master/docs/examples/using-sqlite.md#cmake


cmake_minimum_required(VERSION 3.6)

# Orbit directories
set(ORBIT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(EXTERN_ROOT "${ORBIT_ROOT}/external")
set(ORBIT_UTILS "${ORBIT_ROOT}/OrbitUtils")

# Orbit utils (PrintVars, PrintAllVars...)
include(${ORBIT_UTILS}/utils.cmake)

# Vcpkg toolchain
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${EXTERN_ROOT}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# Linux
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Stop compilation on first error
if(LINUX)
add_definitions(-Wfatal-errors)
endif()

set( LIB_EXT "a" )
if(WIN32)
set( LIB_EXT "lib" )
endif()


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(NAME orbitProfiler)
project(${NAME})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#Output pdbs
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

include(CheckIncludeFiles)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckLibraryExists)
#include(CheckPrototypeExists)
include(CheckStructHasMember)

#pkg_check_modules(QT5 Qt5Core REQUIRED)
#find_package(Qt5Core)

#find_package(LLVM REQUIRED CONFIG)
#message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
#message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

get_filename_component(EXTERN_ROOT ${CMAKE_SOURCE_DIR}/external REALPATH)

if(WIN32)
#find_package(unofficial-breakpad REQUIRED)
find_library(BREAKPAD_LIB libbreakpad.lib)
PrintVars(BREAKPAD_LIB)
endif()
if(LINUX)
find_library(BREAKPAD_LIB liblibbreakpad.a)
PrintVars(BREAKPAD_LIB)
endif()

#pkg_check_modules(GLEW glew REQUIRED)
find_package(glew REQUIRED)
PrintVars(GLEW)

#find_package(imgui REQUIRED)
find_path(IMGUI_INCLUDE_DIR imgui.h) 
find_library(IMGUI_LIB imgui.lib)
PrintVars(IMGUI)

#pkg_check_modules(ASIO asio REQUIRED)
#find_package(asio)
find_path(ASIO_INCLUDE_DIR asio.hpp) 
PrintVars(ASIO)

#add_definitions(-DASIO_HAS_STD_CHRONO)
add_definitions(-DASIO_DISABLE_BOOST_DATE_TIME)
include_directories(${ASIO_INCLUDE_DIR})

#pkg_check_modules(CURL libcurl REQUIRED)
#find_package(curl REQUIRED)
find_path(CURL_INCLUDE_DIR curl.h PATH_SUFFIXES capstone)
if(LINUX)
find_library(CURL_LIBRARIES libcurl.a) 
PrintVars(CURL)
endif()

#pkg_check_modules(CEREAL cereal REQUIRED)
find_package(cereal REQUIRED)
PrintVars(CEREAL)

#find_package(capstone REQUIRED)
find_path(CAPSTONE_INCLUDE_DIR capstone.h PATH_SUFFIXES capstone)
if(WIN32)
find_library(CAPSTONE_LIBRARIES capstone.lib
HINTS "external/vcpkg/packages/capstone_x64-windows-static/lib")
endif()
if(LINUX)
find_library(CAPSTONE_LIBRARIES libcapstone.a )
endif()
PrintVars(CAPSTONE)

if(WIN32)
set(VSInstallDir ${CMAKE_GENERATOR_INSTANCE})
set(DIASDKDir "${CMAKE_GENERATOR_INSTANCE}/DIA SDK" CACHE PATH "")
message(STATUS "DIASDKDir: ${DIASDKDir}")
endif()

function(AppendToVar var val)
    set("${var}" "${${var}} ${val}" PARENT_SCOPE)
endfunction()

#if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
#	AppendToVar(CMAKE_CXX_FLAGS "/std:c++17")
#else()
#	AppendToVar(CMAKE_CXX_FLAGS "-std=c++17")
#endif()

if(WIN32)
# Set preprocessor defines
add_definitions(-DNOMINMAX)
add_definitions(-D_WIN32_WINNT=0x0700)
add_definitions(-DNTDDI_VERSION=0x06030000)
endif()
add_definitions(-DUNICODE -D_UNICODE)

#add_definitions(-D_SCL_SECURE_NO_WARNINGS)
#add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Clang specific stuff
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    AppendToVar(CMAKE_CXX_FLAGS "-Wno-switch-enum")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	AppendToVar(CMAKE_CXX_FLAGS "/W4")
	AppendToVar(CMAKE_CXX_FLAGS "/wd4100")
	AppendToVar(CMAKE_CXX_FLAGS "/wd4245")
	AppendToVar(CMAKE_CXX_FLAGS "/wd4244")
	AppendToVar(CMAKE_CXX_FLAGS "/wd4481")
	AppendToVar(CMAKE_CXX_FLAGS "/wd4201")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	AppendToVar(CMAKE_CXX_FLAGS "-Wall")
    AppendToVar(CMAKE_CXX_FLAGS "-Wextra")
    #AppendToVar(CMAKE_CXX_FLAGS "-Wundef")
    AppendToVar(CMAKE_CXX_FLAGS "-Wno-unused-parameter")
    AppendToVar(CMAKE_CXX_FLAGS "-Wno-unused-local-typedef")
    AppendToVar(CMAKE_CXX_FLAGS "-Wno-missing-field-initializers")
endif()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

if(WIN32)
add_subdirectory(OrbitAsm)
add_subdirectory(OrbitDll)
endif()

add_subdirectory(OrbitCore)
add_subdirectory(OrbitGl)
add_subdirectory(OrbitQt)
