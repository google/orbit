# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(Introspection)

add_library(Introspection STATIC)

target_include_directories(Introspection PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(Introspection PUBLIC
        include/Introspection/Introspection.h)

target_sources(Introspection PRIVATE
        Introspection.cpp)

target_link_libraries(Introspection PUBLIC
        ApiInterface
        ApiUtils
        OrbitBase)

add_executable(IntrospectionTests)

target_sources(IntrospectionTests PRIVATE
        IntrospectionSmokeTest.cpp)

target_link_libraries(IntrospectionTests PRIVATE
        Introspection
        OrbitBase
        GTest_Main
        absl::base
        absl::flat_hash_map
        absl::synchronization)

register_test(IntrospectionTests)
