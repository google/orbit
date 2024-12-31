# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(SshQtTestUtils)
add_library(SshQtTestUtils STATIC)

target_include_directories(SshQtTestUtils PUBLIC include/)
target_link_libraries(SshQtTestUtils PUBLIC
  OrbitBase
  OrbitSsh
  OrbitSshQt
  QtTestUtils
  TestUtils
  absl::strings
  Qt5::Test)

target_sources(SshQtTestUtils PRIVATE
  KillProcessListeningOnTcpPort.cpp
  ParsePortNumberFromSocatOutput.cpp)

target_sources(SshQtTestUtils PUBLIC
  include/SshQtTestUtils/KillProcessListeningOnTcpPort.h
  include/SshQtTestUtils/ParsePortNumberFromSocatOutput.h
  include/SshQtTestUtils/SshSessionTest.h
  include/SshQtTestUtils/SshTestFixture.h)

add_executable(SshQtTestUtilsTests
  ParsePortNumberFromSocatOutputTest.cpp)

target_link_libraries(SshQtTestUtilsTests PRIVATE SshQtTestUtils TestUtils GTest_Main)
register_test(SshQtTestUtilsTests)