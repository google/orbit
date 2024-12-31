# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(CommandLineUtils CXX)

add_library(CommandLineUtils STATIC)

target_sources(
    CommandLineUtils
    PRIVATE CommandLineUtils.cpp
)

target_sources(
  CommandLineUtils
  PRIVATE include/CommandLineUtils/CommandLineUtils.h
)

target_link_libraries(CommandLineUtils PUBLIC Qt5::Core ClientFlags absl::flags absl::flat_hash_set)
target_include_directories(CommandLineUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(CommandLineUtilsTests)

target_sources(CommandLineUtilsTests PRIVATE
               CommandLineUtilsTest.cpp)

target_link_libraries(
  CommandLineUtilsTests
  PRIVATE CommandLineUtils
          GTest_Main)

register_test(CommandLineUtilsTests)