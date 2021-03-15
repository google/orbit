# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(UserSpaceInstrumentation)

add_library(UserSpaceInstrumentation STATIC)

target_compile_options(UserSpaceInstrumentation PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(UserSpaceInstrumentation PUBLIC cxx_std_17)

target_include_directories(UserSpaceInstrumentation PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(UserSpaceInstrumentation PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(UserSpaceInstrumentation PUBLIC
        include/UserSpaceInstrumentation/Attach.h
        include/UserSpaceInstrumentation/RegisterState.h)

target_sources(UserSpaceInstrumentation PRIVATE
        AccessTraceesMemory.cpp
        AccessTraceesMemory.h
        AllocateInTracee.cpp
        AllocateInTracee.h
        Attach.cpp
        InjectLibraryInTracee.cpp
        InjectLibraryInTracee.h
        MachineCode.cpp
        MachineCode.h
        RegisterState.cpp)

target_link_libraries(UserSpaceInstrumentation PUBLIC
        LinuxTracing
        OrbitBase
        CONAN_PKG::abseil)

# This test lib is merely used in UserSpaceInstrumentationTests below. The
# binary libUserSpaceInstrumentationTestLib.so created from this target is used
# to test the injection mechanism.
add_library(UserSpaceInstrumentationTestLib SHARED)

target_compile_options(UserSpaceInstrumentationTestLib PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(UserSpaceInstrumentationTestLib PUBLIC cxx_std_17)

target_include_directories(UserSpaceInstrumentationTestLib PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(UserSpaceInstrumentationTestLib PRIVATE
        UserSpaceInstrumentationTestLib.cpp
        UserSpaceInstrumentationTestLib.h)

add_executable(UserSpaceInstrumentationTests)

target_compile_options(UserSpaceInstrumentationTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(UserSpaceInstrumentationTests PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(UserSpaceInstrumentationTests PRIVATE
        AccessTraceesMemoryTest.cpp
        AllocateInTraceeTest.cpp
        AttachTest.cpp
        InjectLibraryInTraceeTest.cpp
        MachineCodeTest.cpp
        RegisterStateTest.cpp
        TestProcess.cpp
        TestProcess.h)

target_link_libraries(UserSpaceInstrumentationTests PRIVATE
        LinuxTracing
        UserSpaceInstrumentation
        CONAN_PKG::abseil
        GTest::GTest
        GTest::Main)

register_test(UserSpaceInstrumentationTests)
