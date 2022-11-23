# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(RemoteSymbolProvider)
add_library(RemoteSymbolProvider STATIC)

target_sources(RemoteSymbolProvider PRIVATE
    MicrosoftSymbolServerSymbolProvider.cpp)

target_sources(RemoteSymbolProvider PUBLIC
    include/RemoteSymbolProvider/MicrosoftSymbolServerSymbolProvider.h)

target_include_directories(RemoteSymbolProvider PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(RemoteSymbolProvider PUBLIC
    Http
    OrbitBase
    QtUtils
    SymbolProvider
    Symbols)

add_executable(RemoteSymbolProviderTests)

target_sources(RemoteSymbolProviderTests PRIVATE
    MicrosoftSymbolServerSymbolProviderIntegrationTest.cpp
    MicrosoftSymbolServerSymbolProviderTest.cpp)

target_link_libraries(RemoteSymbolProviderTests PRIVATE
    GrpcProtos
    ObjectUtils
    RemoteSymbolProvider
    TestUtils
    Qt5::Core
    GTest::QtCoreMain)

register_test(RemoteSymbolProviderTests)