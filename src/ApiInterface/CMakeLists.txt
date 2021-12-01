# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(ApiInterface INTERFACE)

target_sources(ApiInterface INTERFACE
        include/ApiInterface/Orbit.h)

target_include_directories(ApiInterface INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include)


# This needs to be an executable because we also need to verify linking.
add_executable(ApiHeaderBuildsInCTest)

target_include_directories(ApiHeaderBuildsInCTest PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include/ApiInterface)

target_sources(ApiHeaderBuildsInCTest PRIVATE
        BuildInCMain.c
        BuildInCWithInstantiateInAnotherFile.c
        BuildInCWithInstantiateInThisFile.c)
