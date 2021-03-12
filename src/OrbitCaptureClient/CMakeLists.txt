# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitCaptureClient)
add_library(OrbitCaptureClient STATIC)

target_compile_options(OrbitCaptureClient PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitCaptureClient PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitCaptureClient PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitCaptureClient PUBLIC
        include/OrbitCaptureClient/ApiEventProcessor.h
        include/OrbitCaptureClient/CaptureClient.h
        include/OrbitCaptureClient/CaptureListener.h
        include/OrbitCaptureClient/CaptureEventProcessor.h
        include/OrbitCaptureClient/GpuQueueSubmissionProcessor.h)

target_sources(OrbitCaptureClient PRIVATE
        ApiEventProcessor.cpp
        CaptureClient.cpp
        CaptureEventProcessor.cpp
        GpuQueueSubmissionProcessor.cpp)

target_link_libraries(OrbitCaptureClient PUBLIC
        GrpcProtos
        OrbitCore)

add_fuzzer(CaptureEventProcessorProcessEventsFuzzer CaptureEventProcessorProcessEventsFuzzer.cpp)
target_link_libraries(
  CaptureEventProcessorProcessEventsFuzzer
  PRIVATE OrbitCaptureClient CONAN_PKG::libprotobuf-mutator)

add_executable(OrbitCaptureClientTests)

target_compile_options(OrbitCaptureClientTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitCaptureClientTests PRIVATE
        ApiEventProcessorTest.cpp
        CaptureEventProcessorTest.cpp)

target_link_libraries(
        OrbitCaptureClientTests PRIVATE
        OrbitCaptureClient
        GTest::Main)

register_test(OrbitCaptureClientTests)
