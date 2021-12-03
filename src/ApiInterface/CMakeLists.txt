# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(ApiInterface INTERFACE)

target_sources(ApiInterface INTERFACE
        include/ApiInterface/Orbit.h)

target_include_directories(ApiInterface INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include)


# These need to be executables because we also need to verify linking.
add_executable(ApiHeaderBuildsOn32BitTest)

set_target_properties(ApiHeaderBuildsOn32BitTest PROPERTIES
        COMPILE_OPTIONS "-m32" LINK_OPTIONS "-m32")

target_include_directories(ApiHeaderBuildsOn32BitTest PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include/ApiInterface)

target_sources(ApiHeaderBuildsOn32BitTest PRIVATE
        BuildInCppMain.cpp
        BuildInCppWithInstantiateInAnotherFile.cpp
        BuildInCppWithInstantiateInThisFile.cpp
        UseOrbitApiForBuildTests.h)


add_executable(ApiHeaderBuildsInCTest)

target_include_directories(ApiHeaderBuildsInCTest PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include/ApiInterface)

target_sources(ApiHeaderBuildsInCTest PRIVATE
        BuildInCMain.c
        BuildInCWithInstantiateInAnotherFile.c
        BuildInCWithInstantiateInThisFile.c
        UseOrbitApiForBuildTests.h)


add_executable(ApiHeaderBuildsInCOn32BitTest)

set_target_properties(ApiHeaderBuildsInCOn32BitTest PROPERTIES
        COMPILE_OPTIONS "-m32" LINK_OPTIONS "-m32")

target_include_directories(ApiHeaderBuildsInCOn32BitTest PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include/ApiInterface)

target_sources(ApiHeaderBuildsInCOn32BitTest PRIVATE
        BuildInCMain.c
        BuildInCWithInstantiateInAnotherFile.c
        BuildInCWithInstantiateInThisFile.c
        UseOrbitApiForBuildTests.h)
