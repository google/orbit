# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitVersion)

add_library(OrbitVersion STATIC)

target_compile_options(OrbitVersion PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(OrbitVersion PUBLIC cxx_std_17)

target_include_directories(OrbitVersion PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(OrbitVersion PRIVATE 
        include/OrbitVersion/OrbitVersion.h)

include("${CMAKE_SOURCE_DIR}/cmake/version.cmake")
GenerateVersionFile("${CMAKE_CURRENT_BINARY_DIR}/OrbitVersion.cpp"
                    "${CMAKE_CURRENT_SOURCE_DIR}/OrbitVersion.cpp.in" OrbitVersion)
target_sources(OrbitVersion PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/OrbitVersion.cpp")

