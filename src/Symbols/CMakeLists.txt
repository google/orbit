# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(Symbols STATIC)

target_sources(Symbols PRIVATE
        SymbolHelper.cpp
        SymbolUtils.cpp)
target_sources(Symbols PUBLIC
        include/Symbols/MockSymbolCache.h
        include/Symbols/SymbolCacheInterface.h
        include/Symbols/SymbolHelper.h
        include/Symbols/SymbolUtils.h)

target_include_directories(Symbols PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(Symbols PUBLIC
        GrpcProtos
        Introspection
        ObjectUtils
        OrbitBase
        SymbolProvider)

add_executable(SymbolsTests)
target_sources(SymbolsTests PRIVATE
        SymbolHelperTest.cpp
        SymbolUtilsTest.cpp)
target_link_libraries(SymbolsTests PRIVATE Symbols TestUtils GTest_Main)
register_test(SymbolsTests)
