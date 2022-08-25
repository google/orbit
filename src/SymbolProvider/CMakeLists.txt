# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(SymbolProvider)
add_library(SymbolProvider INTERFACE)

target_sources(SymbolProvider INTERFACE
        include/SymbolProvider/ModuleIdentifier.h)

target_include_directories(SymbolProvider INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include)

add_executable(SymbolProviderTests)

target_sources(SymbolProviderTests PRIVATE 
        ModuleIdentifierTest.cpp)

target_link_libraries(SymbolProviderTests PRIVATE 
        SymbolProvider 
        GTest::Main)
    
register_test(SymbolProviderTests)
