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
        include/CaptureService/CaptureServiceImpl.h
        include/CaptureService/CaptureStartStopListener.h)

target_sources(CaptureService PRIVATE
        CaptureServiceImpl.cpp
        MemoryInfoHandler.cpp
        MemoryInfoHandler.h
        TracingHandler.cpp
        TracingHandler.h)

target_link_libraries(CaptureService PUBLIC
        ApiLoader
        ApiUtils
        CaptureEventProcessor
        CaptureEventProcessor
        GrpcProtos
        Introspection
        LinuxTracing
        MemoryTracing
        OrbitBase
        OrbitVersion
        UserSpaceInstrumentation)

