cmake_minimum_required(VERSION 3.15)

# Note: (Windows + vcpkg) to build and use x64: a) set
# VCPKG_DEFAULT_TRIPLET=x64-windows, then install all required dependencies b)
# invoke cmake for 64 bit platform, as follows: cmake
# -DCMAKE_GENERATOR_PLATFORM=x64 .. or, i.e.: cmake -G "Visual Studio 15 2017
# Win64" .. This assumes CMAKE_TOOLCHAIN_FILE is correctly set, in accordance
# with https://github.com/Microsoft/vcpkg/blob/master/docs/examples/using-
# sqlite.md#cmake

set(CMAKE_TOOLCHAIN_FILE
    "${CMAKE_CURRENT_LIST_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Default CMake toolchain file")

project(Orbit C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(NOT WIN32)
  find_package(X11 REQUIRED)
  find_package(OpenSSL REQUIRED)
  find_package(unwindstack REQUIRED)
endif()

find_package(Threads REQUIRED)
find_package(cereal REQUIRED)
find_package(xxHash REQUIRED)
find_package(multicore REQUIRED)
find_package(concurrentqueue REQUIRED)
find_package(Freetype REQUIRED)
find_package(freetype-gl REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(imgui REQUIRED)
find_package(CURL REQUIRED)
find_package(oqpi REQUIRED)
find_package(capstone REQUIRED)
find_package(GLUT REQUIRED)
find_package(unofficial-abseil CONFIG REQUIRED)

# Hack for freelgut:
# freeglut accidentally links against libxrandr when the headers of
# libxrandr are installed on the current system. Hopefully
# this will be fixed in a later release of either freeglut
# or vcpkg.
if (TARGET X11::Xrandr)
  target_link_libraries(GLUT::GLUT INTERFACE X11::Xrandr)
endif()

find_package(Qt5 COMPONENTS Core Widgets)
find_package(lzma REQUIRED)
find_package(GTest MODULE REQUIRED)

if(WIN32)
  find_package(minhook REQUIRED)
  find_package(unofficial-breakpad CONFIG REQUIRED)
  find_package(DIASDK REQUIRED)
  find_package(DIA2Dump REQUIRED)
  find_package(peparse REQUIRED)

  # Fix the include path in the breakpad target
  get_target_property(INC unofficial::breakpad::libbreakpad
                      INTERFACE_INCLUDE_DIRECTORIES)
  set_target_properties(
    unofficial::breakpad::libbreakpad unofficial::breakpad::libbreakpad_client
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${INC}/google_breakpad")
endif()

# Since the curl packages does not provide cmake targets, we will add one here.
add_library(CURL STATIC IMPORTED GLOBAL)
set_target_properties(CURL PROPERTIES IMPORTED_LOCATION ${CURL_LIBRARIES})
target_include_directories(CURL SYSTEM INTERFACE ${CURK_INCLUDE_DIRS})
if(NOT WIN32)
  target_link_libraries(CURL INTERFACE OpenSSL::SSL)
endif()
add_library(CURL::CURL ALIAS CURL)

# Set preprocessor defines These are only necessary for windows, but we will
# define them on all platforms, to keep the builds similar as possible.
add_definitions(-DNOMINMAX)
add_definitions(-DUNICODE -D_UNICODE)

if(WIN32)
  add_definitions(-DWIN32)

  add_subdirectory(OrbitDll)
  add_subdirectory(OrbitAsm)
else()
  # Hack: Some parts on linux depend on headers in OrbitAsm, but OrbitAsm is not
  # available for Windows.
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
