# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitBase)

add_library(OrbitBase STATIC)

target_compile_options(OrbitBase PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(OrbitBase PUBLIC cxx_std_17)

target_include_directories(OrbitBase PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitBase PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitBase PRIVATE
        include/OrbitBase/Action.h
        include/OrbitBase/Logging.h
        include/OrbitBase/MainThreadExecutor.h
        include/OrbitBase/SafeStrerror.h)

target_sources(OrbitBase PRIVATE
        MainThreadExecutor.cpp
        SafeStrerror.cpp)

target_link_libraries(OrbitBase PUBLIC
        abseil::abseil
        std::filesystem)

add_executable(OrbitBaseTests)

target_sources(OrbitBaseTests PRIVATE
    MainThreadExecutorTest.cpp
)

target_link_libraries(OrbitBaseTests PRIVATE
        OrbitBase
        GTest::GTest
        GTest::Main)

register_test(OrbitBaseTests)

