# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(MemoryTracing)
add_library(MemoryTracing STATIC)

target_include_directories(MemoryTracing PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(MemoryTracing PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(MemoryTracing PUBLIC    
        include/MemoryTracing/MemoryInfoListener.h
        include/MemoryTracing/MemoryInfoProducer.h
        include/MemoryTracing/MemoryTracingUtils.h)

target_sources(MemoryTracing PRIVATE
        MemoryInfoListener.cpp
        MemoryInfoProducer.cpp
        MemoryTracingUtils.cpp)

target_link_libraries(MemoryTracing PUBLIC
        GrpcProtos
        OrbitBase
        absl::flat_hash_map
        absl::time
        absl::str_format
        absl::strings
        absl::synchronization)

add_executable(MemoryTracingTests)

target_sources(MemoryTracingTests PRIVATE 
        MemoryTracingIntegrationTest.cpp
        MemoryTracingUtilsTest.cpp)

target_link_libraries(MemoryTracingTests PRIVATE
        MemoryTracing
        GTest::gtest
        GTest_Main)

register_test(MemoryTracingTests)