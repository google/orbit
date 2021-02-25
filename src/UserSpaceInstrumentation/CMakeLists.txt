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
        Attach.cpp
        RegisterState.cpp)

target_link_libraries(UserSpaceInstrumentation PUBLIC
        LinuxTracing
        OrbitBase
        CONAN_PKG::abseil)

add_executable(UserSpaceInstrumentationTests)

target_compile_options(UserSpaceInstrumentationTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(UserSpaceInstrumentationTests PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(UserSpaceInstrumentationTests PRIVATE
        AttachTest.cpp
        RegisterStateTest.cpp
        TestProcess.cpp)

target_link_libraries(UserSpaceInstrumentationTests PRIVATE
        LinuxTracing
        UserSpaceInstrumentation
        CONAN_PKG::abseil
        GTest::GTest
        GTest::Main)

register_test(UserSpaceInstrumentationTests)
