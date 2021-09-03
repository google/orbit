# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(CaptureService)
add_library(CaptureService STATIC)

target_include_directories(CaptureService PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(CaptureService PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(CaptureService PUBLIC
        include/CaptureService/CaptureEventBuffer.h
        include/CaptureService/CaptureServiceImpl.h
        include/CaptureService/CaptureStartStopListener.h
        include/CaptureService/ProducerEventProcessor.h)

target_sources(CaptureService PRIVATE
        CaptureEventSender.h
        CaptureServiceImpl.cpp
        MemoryInfoHandler.cpp
        MemoryInfoHandler.h
        ProducerEventProcessor.cpp
        TracingHandler.cpp
        TracingHandler.h)

target_link_libraries(CaptureService PUBLIC
        ApiLoader
        ApiUtils
        GrpcProtos
        Introspection
        LinuxTracing
        MemoryTracing
        OrbitBase
        OrbitVersion
        UserSpaceInstrumentation)

add_executable(CaptureServiceTests)

target_sources(CaptureServiceTests PRIVATE
        ProducerEventProcessorTest.cpp)

target_link_libraries(CaptureServiceTests PRIVATE
        TestUtils
        CaptureService
        GTest::Main)

register_test(CaptureServiceTests)
