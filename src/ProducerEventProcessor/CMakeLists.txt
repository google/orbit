# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(ProducerEventProcessor)
add_library(ProducerEventProcessor STATIC)

target_include_directories(ProducerEventProcessor PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(ProducerEventProcessor PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(ProducerEventProcessor PUBLIC
        include/ProducerEventProcessor/ClientCaptureEventCollector.h
        include/ProducerEventProcessor/GrpcClientCaptureEventCollector.h
        include/ProducerEventProcessor/ProducerEventProcessor.h)

target_sources(ProducerEventProcessor PRIVATE
        GrpcClientCaptureEventCollector.cpp
        ProducerEventProcessor.cpp)

target_link_libraries(ProducerEventProcessor PUBLIC
        CaptureFile
        GrpcProtos
        Introspection
        OrbitBase)

add_executable(ProducerEventProcessorTests)

target_sources(ProducerEventProcessorTests PRIVATE
        GrpcClientCaptureEventCollectorTest.cpp
        ProducerEventProcessorTest.cpp)

target_link_libraries(ProducerEventProcessorTests PRIVATE
        TestUtils
        ProducerEventProcessor
        GTest_Main)

register_test(ProducerEventProcessorTests)
