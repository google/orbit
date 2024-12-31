# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitPaths)

add_library(OrbitPaths STATIC)

target_include_directories(OrbitPaths PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(OrbitPaths PUBLIC
        include/OrbitPaths/Paths.h)

target_sources(OrbitPaths PRIVATE
        Paths.cpp)

target_link_libraries(OrbitPaths PUBLIC
        OrbitBase
        absl::flags
        absl::str_format)

add_executable(OrbitPathsTests)

target_sources(OrbitPathsTests PRIVATE
        PathsTest.cpp)

target_link_libraries(OrbitPathsTests PRIVATE
        OrbitPaths
        TestUtils
        GTest_Main)

register_test(OrbitPathsTests)
