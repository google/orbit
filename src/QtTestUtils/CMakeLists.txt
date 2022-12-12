# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(QtTestUtils)
add_library(QtTestUtils INTERFACE)

target_include_directories(QtTestUtils INTERFACE include/)
target_link_libraries(QtTestUtils INTERFACE
  OrbitBase
  Qt5::Core
  Qt5::Test)

target_sources(QtTestUtils PUBLIC include/QtTestUtils/WaitFor.h)

add_executable(QtTestUtilsTests)
target_sources(QtTestUtilsTests PRIVATE WaitForTest.cpp)
target_link_libraries(QtTestUtilsTests PRIVATE TestUtils QtTestUtils GTest::QtCoreMain)
register_test(QtTestUtilsTests)
