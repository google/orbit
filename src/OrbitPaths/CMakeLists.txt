# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitPaths)

add_library(OrbitPaths STATIC)

target_compile_options(OrbitPaths PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(OrbitPaths PUBLIC cxx_std_17)

target_include_directories(OrbitPaths PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(OrbitPaths PUBLIC
        include/OrbitPaths/Paths.h)

target_sources(OrbitPaths PRIVATE
        Paths.cpp)

target_link_libraries(OrbitPaths PUBLIC
        OrbitBase
        CONAN_PKG::abseil)

add_executable(OrbitPathsTests)

target_compile_options(OrbitPathsTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitPathsTests PRIVATE
        PathsTest.cpp)

target_link_libraries(OrbitPathsTests PRIVATE
        OrbitPaths
        GTest::Main)

register_test(OrbitPathsTests)
