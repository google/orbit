# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ClientSymbols)
add_library(ClientSymbols STATIC)

target_include_directories(ClientSymbols PUBLIC include/)
target_link_libraries(ClientSymbols PUBLIC 
        CONAN_PKG::abseil        
        Qt5::Core)

target_sources(ClientSymbols PUBLIC 
        include/ClientSymbols/PersistentStorageManager.h
        include/ClientSymbols/QSettingsBasedStorageManager.h)

target_sources(ClientSymbols PRIVATE 
        QSettingsBasedStorageManager.cpp)

add_executable(ClientSymbolsTests)
target_sources(ClientSymbolsTests PRIVATE 
        QSettingsBasedStorageManagerTest.cpp)

target_link_libraries(ClientSymbolsTests PRIVATE 
        ClientSymbols
        GTest::QtCoreMain)

register_test(ClientSymbolsTests)
