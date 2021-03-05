# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(MemoryTracing)
add_library(MemoryTracing STATIC)

target_compile_options(MemoryTracing PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(MemoryTracing PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(MemoryTracing PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(MemoryTracing PUBLIC
        include/MemoryTracing/MemoryInfoListener.h
        include/MemoryTracing/MemoryInfoProducer.h)

target_sources(MemoryTracing PRIVATE
        MemoryInfoProducer.cpp
        MemoryTracingUtils.cpp
        MemoryTracingUtils.h)

target_link_libraries(MemoryTracing PUBLIC
        GrpcProtos
        OrbitBase
        OrbitCore
        CONAN_PKG::abseil)

add_executable(MemoryTracingTests)

target_compile_options(MemoryTracingTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(MemoryTracingTests PRIVATE MemoryTracingUtilsTest.cpp)

target_link_libraries(MemoryTracingTests PRIVATE
        MemoryTracing
        GTest::GTest
        GTest::Main)

register_test(MemoryTracingTests)