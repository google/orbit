# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(WindowsUtils)

add_library(WindowsUtils STATIC)

target_compile_definitions(WindowsUtils PUBLIC WIN32_LEAN_AND_MEAN)
target_compile_definitions(WindowsUtils PUBLIC INITGUID)

target_include_directories(WindowsUtils PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(WindowsUtils PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(WindowsUtils PUBLIC
        include/WindowsUtils/AdjustTokenPrivilege.h
        include/WindowsUtils/BusyLoopLauncher.h
        include/WindowsUtils/BusyLoopUtils.h
        include/WindowsUtils/CreateProcess.h
        include/WindowsUtils/Debugger.h
        include/WindowsUtils/DllInjection.h
        include/WindowsUtils/FindDebugSymbols.h
        include/WindowsUtils/ListModules.h
        include/WindowsUtils/ListThreads.h
        include/WindowsUtils/OpenProcess.h
        include/WindowsUtils/PathConverter.h
        include/WindowsUtils/ProcessLauncher.h
        include/WindowsUtils/ProcessList.h
        include/WindowsUtils/ReadProcessMemory.h
        include/WindowsUtils/SafeHandle.h
        include/WindowsUtils/WriteProcessMemory.h)

target_sources(WindowsUtils PRIVATE
        AdjustTokenPrivilege.cpp
        BusyLoopLauncher.cpp
        BusyLoopUtils.cpp
        CreateProcess.cpp
        Debugger.cpp
        DllInjection.cpp
        FindDebugSymbols.cpp
        ListModules.cpp
        ListThreads.cpp
        OpenProcess.cpp
        PathConverter.cpp
        ProcessLauncher.cpp
        ProcessList.cpp
        ReadProcessMemory.cpp
        SafeHandle.cpp
        WriteProcessMemory.cpp)

target_link_libraries(WindowsUtils PUBLIC
        ObjectUtils
        OrbitBase
        absl::base
        absl::flat_hash_map
        absl::str_format
        absl::strings
        absl::synchronization)

add_executable(WindowsUtilsTests)

target_sources(WindowsUtilsTests PRIVATE
        BusyLoopLauncherTest.cpp
        CreateProcessTest.cpp
        DebuggerTest.cpp
        DllInjectionTest.cpp
        FindDebugSymbolsTest.cpp
        ListModulesTest.cpp
        ListThreadsTest.cpp
        OpenProcessTest.cpp
        PathConverterTest.cpp
        ProcessLauncherTest.cpp
        ProcessListTest.cpp
        ReadProcessMemoryTest.cpp
        SafeHandleTest.cpp
        WriteProcessMemoryTest.cpp)

target_link_libraries(WindowsUtilsTests PRIVATE
        TestUtils
        WindowsUtils
        GTest::gtest
        GTest_Main)

SET_TARGET_PROPERTIES(WindowsUtilsTests PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")

register_test(WindowsUtilsTests)


