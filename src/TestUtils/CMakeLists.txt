# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(TestUtils STATIC)
target_sources(TestUtils PUBLIC
  include/TestUtils/ContainerHelpers.h
  include/TestUtils/SaveRangeFromArg.h
  include/TestUtils/TemporaryDirectory.h
  include/TestUtils/TemporaryFile.h
  include/TestUtils/TestUtils.h)

target_sources(TestUtils PRIVATE
  TemporaryDirectory.cpp
  TemporaryFile.cpp)

target_include_directories(TestUtils PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(TestUtils PUBLIC
  OrbitBase
  absl::flat_hash_map
  absl::flat_hash_set
  absl::strings
  absl::str_format
  GTest_Main
  gtest
  gmock)

add_executable(TestUtilsTests
  ContainerHelpersTest.cpp
  SaveRangeFromArgTest.cpp
  TemporaryDirectoryTest.cpp
  TemporaryFileTest.cpp
  TestUtilsTest.cpp)
target_link_libraries(TestUtilsTests PRIVATE TestUtils GTest_Main)
register_test(TestUtilsTests)