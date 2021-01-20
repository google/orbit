# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(FramePointerValidator)

add_library(FramePointerValidator STATIC)
target_compile_options(FramePointerValidator PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(FramePointerValidator PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(FramePointerValidator PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(FramePointerValidator PUBLIC
        include/FramePointerValidator/FramePointerValidator.h
        include/FramePointerValidator/FunctionFramePointerValidator.h)

target_sources(FramePointerValidator PRIVATE
        FramePointerValidator.cpp
        FunctionFramePointerValidator.cpp)

target_link_libraries(FramePointerValidator PUBLIC
        GrpcProtos
        OrbitBase
        CONAN_PKG::abseil
        CONAN_PKG::capstone)

add_executable(FramePointerValidatorTests)

target_compile_options(FramePointerValidatorTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(FramePointerValidatorTests PRIVATE
        FramePointerValidatorTest.cpp
        FunctionFramePointerValidatorTest.cpp)

target_link_libraries(FramePointerValidatorTests PRIVATE
        FramePointerValidator
        ElfUtils
        OrbitBase
        GTest::Main
        CONAN_PKG::abseil
        CONAN_PKG::capstone)

register_test(FramePointerValidatorTests)
