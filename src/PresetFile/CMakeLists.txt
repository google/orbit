# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(PresetFile)
add_library(PresetFile STATIC)

target_sources(
  PresetFile
  PUBLIC include/PresetFile/PresetFile.h)

target_sources(
  PresetFile
  PRIVATE PresetFile.cpp)

target_include_directories(PresetFile PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(
  PresetFile
  PUBLIC OrbitBase
         ClientProtos
         absl::strings
         protobuf::protobuf)

add_executable(PresetFileTests)

target_sources(PresetFileTests PRIVATE
        PresetFileTest.cpp)

target_link_libraries(
  PresetFileTests
  PRIVATE PresetFile
          TestUtils
          GTest_Main)

register_test(PresetFileTests)
