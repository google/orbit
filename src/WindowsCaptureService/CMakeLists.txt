# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(WindowsCaptureService)
add_library(WindowsCaptureService STATIC)

target_include_directories(WindowsCaptureService PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(WindowsCaptureService PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(WindowsCaptureService PUBLIC
        include/WindowsCaptureService/CaptureServiceImpl.h
        include/WindowsCaptureService/CaptureStartStopListener.h)

target_sources(WindowsCaptureService PRIVATE
        CaptureServiceImpl.cpp
        TracingHandler.cpp
        TracingHandler.h)

target_link_libraries(WindowsCaptureService PUBLIC
        ApiLoader
        ApiUtils
        CaptureEventProcessor
        GrpcProtos
        Introspection
        LinuxTracing
        OrbitBase
        OrbitVersion
        WindowsTracing)

#add_executable(WindowsCaptureServiceTests)

# target_sources(WindowsCaptureServiceTests PRIVATE
#         ProducerEventProcessorTest.cpp)

# target_link_libraries(WindowsCaptureServiceTests PRIVATE
#         TestUtils
#         WindowsCaptureService
#         GTest::Main)

#register_test(WindowsCaptureServiceTests)
