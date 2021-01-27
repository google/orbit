# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(CaptureFile)
add_library(CaptureFile STATIC)

target_compile_options(CaptureFile PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  CaptureFile
  PUBLIC include/CaptureFile/CaptureFileOutputStream.h)

target_sources(
  CaptureFile
  PRIVATE CaptureFileConstants.h
          CaptureFileOutputStream.cpp)

target_include_directories(CaptureFile PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(
  CaptureFile
  PUBLIC OrbitBase
         GrpcProtos
         CONAN_PKG::protobuf)

add_executable(CaptureFileTests)

target_sources(CaptureFileTests PRIVATE
    CaptureFileOutputStreamTest.cpp
)

target_link_libraries(
  CaptureFileTests
  PRIVATE CaptureFile
          GTest::Main
          CONAN_PKG::abseil)

register_test(CaptureFileTests)
