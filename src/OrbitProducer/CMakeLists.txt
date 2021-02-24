# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitProducer)

add_library(OrbitProducer STATIC)
target_compile_options(OrbitProducer PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitProducer PUBLIC
        include/OrbitProducer/CaptureEventProducer.h
        include/OrbitProducer/FakeProducerSideService.h
        include/OrbitProducer/LockFreeBufferCaptureEventProducer.h)

target_sources(OrbitProducer PRIVATE
        CaptureEventProducer.cpp)

target_include_directories(OrbitProducer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(OrbitProducer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(OrbitProducer PUBLIC
        GrpcProtos
        OrbitBase
        ServiceLib
        concurrentqueue::concurrentqueue
        CONAN_PKG::abseil)

add_executable(OrbitProducerTests)

target_compile_options(OrbitProducerTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitProducerTests PRIVATE
        CaptureEventProducerTest.cpp
        LockFreeBufferCaptureEventProducerTest.cpp)

target_link_libraries(OrbitProducerTests PRIVATE
        OrbitProducer
        GTest::Main)

register_test(OrbitProducerTests)
