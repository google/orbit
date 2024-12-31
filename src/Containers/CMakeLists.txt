# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(Containers)
add_library(Containers INTERFACE)

target_sources(Containers INTERFACE
        include/Containers/BlockChain.h
        include/Containers/ScopeTree.h)

target_include_directories(Containers INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(
  Containers
  INTERFACE Introspection
            OrbitBase)

add_executable(ContainersTests)

target_sources(ContainersTests PRIVATE
        BlockChainTest.cpp
        ScopeTreeTest.cpp)

target_link_libraries(
  ContainersTests
  PRIVATE Containers
          GTest_Main)

register_test(ContainersTests)