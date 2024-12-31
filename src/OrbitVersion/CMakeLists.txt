# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitVersion)

add_library(OrbitVersion STATIC)

target_include_directories(OrbitVersion PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(OrbitVersion PRIVATE 
        include/OrbitVersion/OrbitVersion.h)

target_link_libraries(OrbitVersion PUBLIC absl::str_format)

include("${CMAKE_SOURCE_DIR}/cmake/version.cmake")
GenerateVersionFile("${CMAKE_CURRENT_BINARY_DIR}/OrbitVersion.cpp"
                    "${CMAKE_CURRENT_SOURCE_DIR}/OrbitVersion.cpp.in" OrbitVersion)
target_sources(OrbitVersion PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/OrbitVersion.cpp")

add_executable(OrbitVersionTests)

target_sources(OrbitVersionTests PRIVATE OrbitVersionTest.cpp)

target_link_libraries(OrbitVersionTests PRIVATE
        OrbitVersion
        GTest::gtest
        GTest_Main)

register_test(OrbitVersionTests)
