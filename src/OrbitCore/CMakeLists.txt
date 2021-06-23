# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitCore)
add_library(OrbitCore STATIC)

target_compile_options(OrbitCore PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  OrbitCore
  PUBLIC CoreUtils.h
         Path.h
         StringManager.h)

target_sources(
  OrbitCore
  PRIVATE CoreUtils.cpp
          Path.cpp
          StringManager.cpp)

target_include_directories(OrbitCore PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
  OrbitCore
  PUBLIC OrbitBase
         xxHash::xxHash
         CONAN_PKG::abseil)

if(WIN32)
  target_link_libraries(OrbitCore PRIVATE dbghelp.lib)
  target_compile_definitions(OrbitCore PUBLIC -D_WIN32_WINNT=0x0700)
  target_compile_definitions(OrbitCore PUBLIC -DNTDDI_VERSION=0x06030000)
endif()

target_compile_definitions(OrbitCore PUBLIC WIN32_LEAN_AND_MEAN)
target_compile_definitions(OrbitCore PUBLIC INITGUID)
target_compile_definitions(OrbitCore PUBLIC NOMINMAX)
target_compile_definitions(OrbitCore PUBLIC UNICODE)
target_compile_definitions(OrbitCore PUBLIC _UNICODE)

target_compile_features(OrbitCore PUBLIC cxx_std_11)

add_executable(OrbitCoreTests)

target_compile_options(OrbitCoreTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitCoreTests PRIVATE
    PathTest.cpp
    StringManagerTest.cpp)

target_link_libraries(
  OrbitCoreTests
  PRIVATE OrbitCore
          GTest::Main
          CONAN_PKG::abseil)

register_test(OrbitCoreTests)