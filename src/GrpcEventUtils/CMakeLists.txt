# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(CaptureEventProcessor)
add_library(CaptureEventProcessor STATIC)

target_include_directories(CaptureEventProcessor PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(CaptureEventProcessor PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(CaptureEventProcessor PUBLIC
        include/CaptureEventProcessor/ClientCaptureEventCollector.h
        include/CaptureEventProcessor/GrpcClientCaptureEventCollector.h
        include/CaptureEventProcessor/ProducerEventProcessor.h)

target_sources(CaptureEventProcessor PRIVATE
        GrpcClientCaptureEventCollector.cpp
        ProducerEventProcessor.cpp)

target_link_libraries(CaptureEventProcessor PUBLIC
        GrpcProtos
        Introspection
        OrbitBase)

add_executable(GrpcEventUtilsTests)

target_sources(GrpcEventUtilsTests PRIVATE
        GrpcClientCaptureEventCollectorTest.cpp
        ProducerEventProcessorTest.cpp)

target_link_libraries(GrpcEventUtilsTests PRIVATE
        TestUtils
        CaptureEventProcessor
        GTest::Main)

register_test(GrpcEventUtilsTests)
