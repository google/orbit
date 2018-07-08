cmake_minimum_required(VERSION 3.6)

# Note: (Windows + vcpkg) to build and use x64:
# a) set VCPKG_DEFAULT_TRIPLET=x64-windows, then install all required dependencies
# b) invoke cmake for 64 bit platform, as follows:
#    cmake -DCMAKE_GENERATOR_PLATFORM=x64 ..
# or, i.e.: 
#    cmake -G "Visual Studio 15 2017 Win64" ..
# This assumes CMAKE_TOOLCHAIN_FILE is correctly set, in accordance with
# https://github.com/Microsoft/vcpkg/blob/master/docs/examples/using-sqlite.md#cmake

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(NAME orbitProfiler)
project(${NAME})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#include(FindPkgConfig)

function(PrintVars var)
if(DEFINED ${var}_FOUND)
    message(STATUS "${var}_FOUND=${${var}_FOUND}")
endif()
if(DEFINED ${var}_VERSION)
    message(STATUS "${var}_VERSION=${${var}_VERSION}")
endif()
if(DEFINED ${var}_LIB)
    message(STATUS "${var}_LIB=${${var}_LIB}")
endif()
if(DEFINED ${var}_LIBRARIES)
    message(STATUS "${var}_LIBRARIES=${${var}_LIBRARIES}")
endif()
if(DEFINED ${var}_LIBRARY_DIRS)
    message(STATUS "${var}_LIBRARY_DIRS=${${var}_LIBRARY_DIRS}")
endif()
if(DEFINED ${var}_LDFLAGS)
    message(STATUS "${var}_LDFLAGS=${${var}_LDFLAGS}")
endif()
if(DEFINED ${var}_LDFLAGS_OTHER)
    message(STATUS "${var}_LDFLAGS_OTHER=${${var}_LDFLAGS_OTHER}")
endif()
if(DEFINED ${var}_INCLUDE_DIRS)
    message(STATUS "${var}_INCLUDE_DIRS=${${var}_INCLUDE_DIRS}")
endif()
if(DEFINED ${var}_CFLAGS)
    message(STATUS "${var}_CFLAGS=${${var}_CFLAGS}")
endif()
if(DEFINED ${var}_CFLAGS_OTHER)
    message(STATUS "${var}_CFLAGS_OTHER=${${var}_CFLAGS_OTHER}")
endif()
endfunction()

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

#find_package(unofficial-breakpad REQUIRED)
find_library(BREAKPAD_LIB libbreakpad.lib)
PrintVars(BREAKPAD_LIB)

#pkg_check_modules(FREETYPE freetype2 REQUIRED)
find_package(freetype REQUIRED)
PrintVars(FREETYPE)

#find_package(freeglut REQUIRED)
find_path(FREEGLUT_INCLUDE_DIR GL/freeglut.h)
find_library(FREEGLUT_LIB freeglut_static.lib
    HINTS "external/vcpkg/buildtrees/freeglut/x64-windows-static-rel/lib/Release")
PrintVars(FREEGLUT)

#pkg_check_modules(GLEW glew REQUIRED)
find_package(glew REQUIRED)
PrintVars(GLEW)

find_path(FREETYPE_GL_INCLUDE_DIR freetype-gl.h PATH_SUFFIXES freetype-gl)
find_library(FREETYPE_GL_LIB freetype-gl.lib)
PrintVars(FREETYPE_GL)

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
find_library(CURL_LIBRARIES libcurl) 
#HINTS "external/vcpkg/packages/capstone_x64-windows-static/lib")
PrintVars(CURL)

#pkg_check_modules(CEREAL cereal REQUIRED)
find_package(cereal REQUIRED)
PrintVars(CEREAL)

#find_package(capstone REQUIRED)
find_path(CAPSTONE_INCLUDE_DIR capstone.h PATH_SUFFIXES capstone)
find_library(CAPSTONE_LIBRARIES capstone.lib 
HINTS "external/vcpkg/packages/capstone_x64-windows-static/lib")
PrintVars(CAPSTONE)

#string(REGEX REPLACE "/bin/[^/]*$" "" VCINSTALLDIR "${CMAKE_C_COMPILER}")
#message(STATUS "Guessed MSVC directory: ${VCINSTALLDIR}")
set(VSInstallDir "C:/Program Files (x86)/Microsoft Visual Studio 14.0" CACHE PATH "")
set(DIASDKDir "${VSInstallDir}/DIA SDK" CACHE PATH "")
message(STATUS "DIASDKDir: ${DIASDKDir}")

function(AppendToVar var val)
    set("${var}" "${${var}} ${val}" PARENT_SCOPE)
endfunction()

#if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
#	AppendToVar(CMAKE_CXX_FLAGS "/std:c++17")
#else()
#	AppendToVar(CMAKE_CXX_FLAGS "-std=c++17")
#endif()

# Set preprocessor defines
add_definitions(-DNOMINMAX)
add_definitions(-D_WIN32_WINNT=0x0700)
add_definitions(-DNTDDI_VERSION=0x06030000)
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

add_subdirectory(OrbitAsm)
add_subdirectory(OrbitDll)
add_subdirectory(OrbitCore)
add_subdirectory(OrbitGl)
add_subdirectory(OrbitQt)
