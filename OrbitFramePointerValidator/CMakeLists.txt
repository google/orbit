# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitFramePointerValidator)

add_library(OrbitFramePointerValidator STATIC)
target_compile_options(OrbitFramePointerValidator PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitFramePointerValidator PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitFramePointerValidator PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitFramePointerValidator PUBLIC
        include/OrbitFramePointerValidator/FramePointerValidator.h
        include/OrbitFramePointerValidator/FunctionFramePointerValidator.h)

target_sources(OrbitFramePointerValidator PRIVATE
        FramePointerValidator.cpp
        FunctionFramePointerValidator.cpp)

target_link_libraries(OrbitFramePointerValidator PUBLIC
        OrbitBase
        OrbitProtos
        abseil::abseil
        capstone::capstone)

add_executable(OrbitFramePointerValidatorTests)

target_compile_options(OrbitFramePointerValidatorTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitFramePointerValidatorTests PRIVATE
        FramePointerValidatorTest.cpp
        FunctionFramePointerValidatorTest.cpp)

target_link_libraries(OrbitFramePointerValidatorTests PRIVATE
        OrbitCore
        OrbitFramePointerValidator
        GTest::Main
        abseil::abseil)

register_test(OrbitFramePointerValidatorTests)
