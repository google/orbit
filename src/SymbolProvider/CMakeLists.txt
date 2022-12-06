# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(SymbolProvider)
add_library(SymbolProvider STATIC)

target_sources(SymbolProvider PRIVATE
        StructuredDebugDirectorySymbolProvider.cpp
        SymbolLoadingOutcome.cpp)

target_sources(SymbolProvider PUBLIC
        include/SymbolProvider/ModuleIdentifier.h
        include/SymbolProvider/StructuredDebugDirectorySymbolProvider.h
        include/SymbolProvider/SymbolLoadingOutcome.h
        include/SymbolProvider/SymbolProvider.h)

target_include_directories(SymbolProvider PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(SymbolProvider PUBLIC
        OrbitBase)

add_executable(SymbolProviderTests)

target_sources(SymbolProviderTests PRIVATE 
        ModuleIdentifierTest.cpp
        StructuredDebugDirectorySymbolProviderTest.cpp
        SymbolLoadingOutcomeTest.cpp)

target_link_libraries(SymbolProviderTests PRIVATE 
        SymbolProvider 
        TestUtils
        GTest::Main)
    
register_test(SymbolProviderTests)
