# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ModuleUtils)

add_library(ModuleUtils STATIC)

target_include_directories(ModuleUtils PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(ModuleUtils PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(ModuleUtils PUBLIC
        include/ModuleUtils/ReadLinuxMaps.h
        include/ModuleUtils/ReadLinuxModules.h
        include/ModuleUtils/VirtualAndAbsoluteAddresses.h)

target_sources(ModuleUtils PRIVATE
        VirtualAndAbsoluteAddresses.cpp)

if (NOT WIN32)
target_sources(ModuleUtils PRIVATE
        ReadLinuxMaps.cpp
        ReadLinuxModules.cpp)
endif()

target_link_libraries(ModuleUtils PUBLIC
        GrpcProtos
        ObjectUtils
        OrbitBase
        absl::str_format
        absl::strings)

add_executable(ModuleUtilsTests)

target_sources(ModuleUtilsTests PRIVATE
        VirtualAndAbsoluteAddressesTest.cpp)

if (NOT WIN32)
target_sources(ModuleUtilsTests PRIVATE
        ReadLinuxMapsTest.cpp
        ReadLinuxModulesTest.cpp)
endif()

target_link_libraries(ModuleUtilsTests PRIVATE
        ModuleUtils
        TestUtils
        GTest_Main)

register_test(ModuleUtilsTests)
