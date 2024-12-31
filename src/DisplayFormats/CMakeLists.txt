# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(DisplayFormats)
add_library(DisplayFormats STATIC)

target_include_directories(DisplayFormats PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(DisplayFormats PUBLIC
        include/DisplayFormats/DisplayFormats.h)

target_sources(DisplayFormats PRIVATE
        DisplayFormats.cpp)

target_link_libraries(DisplayFormats PUBLIC
        OrbitBase
        absl::str_format
        absl::time)


add_executable(DisplayFormatsTests)

target_sources(DisplayFormatsTests PRIVATE
        DisplayFormatsTest.cpp)

target_link_libraries(DisplayFormatsTests PRIVATE
        DisplayFormats
        OrbitBase
        GTest_Main)

register_test(DisplayFormatsTests)
