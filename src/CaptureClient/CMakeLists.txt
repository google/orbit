# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(CaptureClient)
add_library(CaptureClient STATIC)

target_include_directories(CaptureClient PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(CaptureClient PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(CaptureClient PUBLIC
        include/CaptureClient/AbstractCaptureListener.h
        include/CaptureClient/ApiEventProcessor.h
        include/CaptureClient/AppInterface.h
        include/CaptureClient/CaptureClient.h
        include/CaptureClient/CaptureListener.h
        include/CaptureClient/CaptureEventProcessor.h
        include/CaptureClient/ClientCaptureOptions.h
        include/CaptureClient/GpuQueueSubmissionProcessor.h
        include/CaptureClient/LoadCapture.h)

target_sources(CaptureClient PRIVATE
        ApiEventProcessor.cpp
        CaptureClient.cpp
        CaptureEventProcessor.cpp
        CompositeEventProcessor.cpp
        GpuQueueSubmissionProcessor.cpp
        LoadCapture.cpp
        SaveToFileEventProcessor.cpp)

target_link_libraries(CaptureClient PUBLIC
        ApiUtils
        CaptureFile
        ClientData
        GrpcProtos
        Introspection)

add_executable(CaptureClientTests)

target_sources(CaptureClientTests PRIVATE
        ApiEventProcessorTest.cpp
        CaptureEventProcessorTest.cpp
        CompositeEventProcessorTest.cpp
        GpuQueueSubmissionProcessorTest.cpp
        MockCaptureListener.h
        SaveToFileEventProcessorTest.cpp)

target_link_libraries(CaptureClientTests PRIVATE
        CaptureClient
        TestUtils
        GTest_Main)

register_test(CaptureClientTests)
