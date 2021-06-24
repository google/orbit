# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(Symbols STATIC)

target_sources(Symbols PRIVATE SymbolHelper.cpp)
target_sources(Symbols PUBLIC include/Symbols/SymbolHelper.h)

target_include_directories(Symbols PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_compile_options(Symbols PRIVATE ${STRICT_COMPILE_FLAGS})
target_compile_features(Symbols PUBLIC cxx_std_17)

target_link_libraries(Symbols PUBLIC
        GrpcProtos
        ObjectUtils
        OrbitBase
        OrbitPaths)


add_executable(SymbolsTests)
target_compile_options(SymbolsTests PRIVATE ${STRICT_COMPILE_FLAGS})
target_sources(SymbolsTests PRIVATE SymbolHelperTest.cpp)
target_link_libraries(SymbolsTests PRIVATE Symbols GTest::Main)
register_test(SymbolsTests)
