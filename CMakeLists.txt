cmake_minimum_required(VERSION 3.15)

# Note: (Windows + vcpkg) to build and use x64: a) set
# VCPKG_DEFAULT_TRIPLET=x64-windows, then install all required dependencies b)
# invoke cmake for 64 bit platform, as follows: cmake
# -DCMAKE_GENERATOR_PLATFORM=x64 .. or, i.e.: cmake -G "Visual Studio 15 2017
# Win64" .. This assumes CMAKE_TOOLCHAIN_FILE is correctly set, in accordance
# with https://github.com/Microsoft/vcpkg/blob/master/docs/examples/using-
# sqlite.md#cmake

project(Orbit C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Threads REQUIRED)
find_package(cereal REQUIRED)
find_package(xxHash REQUIRED)
find_package(multicore REQUIRED)
find_package(concurrentqueue REQUIRED)
find_package(freetype REQUIRED)
find_package(freetype-gl REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glew REQUIRED)
find_package(imgui REQUIRED)
find_package(libcurl REQUIRED)

if(NOT WIN32)
  find_package(X11 REQUIRED)
endif()

find_package(freeglut REQUIRED)

find_package(Qt5 COMPONENTS Core Widgets Network)

if(WIN32)
  find_package(minhook REQUIRED)
endif()

if(WIN32)
  # Set preprocessor defines
  add_definitions(-DNOMINMAX)
  add_definitions(-DUNICODE -D_UNICODE)
endif()

# Clang specific stuff
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  list(APPEND CMAKE_CXX_FLAGS "-Wno-switch-enum")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  list(CMAKE_<LANG>_ARCHIVE_APPEND CMAKE_CXX_FLAGS "/W4")
  list(APPEND CMAKE_CXX_FLAGS "/wd4100")
  list(APPEND CMAKE_CXX_FLAGS "/wd4245")
  list(APPEND CMAKE_CXX_FLAGS "/wd4244")
  list(APPEND CMAKE_CXX_FLAGS "/wd4481")
  list(APPEND CMAKE_CXX_FLAGS "/wd4201")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  list(APPEND CMAKE_CXX_FLAGS "-Wall")
  list(APPEND CMAKE_CXX_FLAGS "-Wextra")
  list(APPEND CMAKE_CXX_FLAGS "-Wno-unused-parameter")
  list(APPEND CMAKE_CXX_FLAGS "-Wno-missing-field-initializers")
endif()

if(WIN32)
  add_subdirectory(OrbitDll)
  add_subdirectory(OrbitAsm)
else()
  # Hack
  add_library(OrbitAsm INTERFACE)
  target_include_directories(OrbitAsm INTERFACE OrbitAsm/)
endif()

add_subdirectory(OrbitCore)
add_subdirectory(OrbitGl)
add_subdirectory(OrbitService)
add_subdirectory(OrbitTest)

if(TARGET Qt5::Core)
  add_subdirectory(OrbitQt)
endif()

if(WIN32)
  # Startup Project
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT
                                                              "OrbitQt")
endif()
