# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(SshQtTestUtils)
add_library(SshQtTestUtils INTERFACE)

target_include_directories(SshQtTestUtils INTERFACE include/)
target_link_libraries(SshQtTestUtils INTERFACE
  OrbitBase
  OrbitSsh
  OrbitSshQt)

target_sources(SshQtTestUtils PUBLIC
  include/SshQtTestUtils/SshSessionTest.h
  include/SshQtTestUtils/SshTestFixture.h
  )