# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(WindowsUtils)

add_library(WindowsUtils STATIC)

target_compile_definitions(WindowsUtils PUBLIC WIN32_LEAN_AND_MEAN)
target_compile_definitions(WindowsUtils PUBLIC INITGUID)

target_compile_features(WindowsUtils PUBLIC cxx_std_17)

target_include_directories(WindowsUtils PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(WindowsUtils PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(WindowsUtils PUBLIC
        include/WindowsUtils/AdjustTokenPrivilege.h
        include/WindowsUtils/DllInjection.h
        include/WindowsUtils/FindDebugSymbols.h
        include/WindowsUtils/ListModules.h
        include/WindowsUtils/ListThreads.h
        include/WindowsUtils/PerformanceCounter.h
        include/WindowsUtils/ProcessList.h
        include/WindowsUtils/ReadProcessMemory.h)

target_sources(WindowsUtils PRIVATE
        AdjustTokenPrivilege.cpp
        DllInjection.cpp
        FindDebugSymbols.cpp
        ListModules.cpp
        ListThreads.cpp
        ProcessList.cpp
        ReadProcessMemory.cpp)

target_link_libraries(WindowsUtils PUBLIC
        ObjectUtils
        OrbitBase
        CONAN_PKG::abseil)

add_executable(WindowsUtilsTests)

target_sources(WindowsUtilsTests PRIVATE
        DllInjectionTest.cpp
        FindDebugSymbolsTest.cpp
        ListModulesTest.cpp
        ListThreadsTest.cpp
        ProcessListTest.cpp
        ReadProcessMemoryTest.cpp)

target_link_libraries(WindowsUtilsTests PRIVATE
        TestUtils
        WindowsUtils
        GTest::GTest
        GTest::Main)

SET_TARGET_PROPERTIES(WindowsUtilsTests PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")

register_test(WindowsUtilsTests)


