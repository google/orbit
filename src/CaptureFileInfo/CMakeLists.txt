# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(CaptureFileInfo)
add_library(CaptureFileInfo STATIC)

target_compile_options(CaptureFileInfo PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  CaptureFileInfo
  PUBLIC  include/CaptureFileInfo/CaptureFileInfo.h)

target_sources(
  CaptureFileInfo
  PRIVATE CaptureFileInfo.cpp)

target_include_directories(CaptureFileInfo PUBLIC include/)

target_link_libraries(
  CaptureFileInfo
  PUBLIC  Qt5::Core)

add_executable(CaptureFileInfoTests)

target_compile_options(CaptureFileInfoTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  CaptureFileInfoTests 
  PRIVATE CaptureFileInfoTest.cpp)

target_link_libraries(
  CaptureFileInfoTests
  PRIVATE CaptureFileInfo 
          OrbitBase
          GTest::Main)

add_custom_command(TARGET CaptureFileInfoTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/testdata
  $<TARGET_FILE_DIR:CaptureFileInfoTests>/testdata)

register_test(CaptureFileInfoTests)