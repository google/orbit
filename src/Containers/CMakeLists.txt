# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(Containers)
add_library(Containers STATIC)

target_sources(Containers PUBLIC
        include/Containers/BlockChain.h
        include/Containers/ScopeTree.h)

target_include_directories(Containers PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(
  Containers
  PUBLIC Introspection
         OrbitBase)

add_executable(ContainersTests)

target_sources(ContainersTests PRIVATE
        BlockChainTest.cpp
        ScopeTreeTest.cpp)

target_link_libraries(
  ContainersTests
  PRIVATE Containers
          GTest::Main)

register_test(ContainersTests)