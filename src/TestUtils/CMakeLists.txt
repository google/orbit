# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(TestUtils INTERFACE)
target_sources(TestUtils INTERFACE include/TestUtils/TestUtils.h)
target_include_directories(TestUtils INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(TestUtils INTERFACE CONAN_PKG::abseil GTest::GTest)

add_executable(TestUtilsTests TestUtilsTest.cpp)
target_link_libraries(TestUtilsTests PRIVATE TestUtils GTest::Main)
register_test(TestUtilsTests)