# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(LinuxCaptureService)
add_library(LinuxCaptureService STATIC)

target_include_directories(LinuxCaptureService PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(LinuxCaptureService PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(LinuxCaptureService PUBLIC
        include/LinuxCaptureService/LinuxCaptureService.h)

target_sources(LinuxCaptureService PRIVATE
        LinuxCaptureService.cpp
        MemoryInfoHandler.cpp
        MemoryInfoHandler.h
        TracingHandler.cpp
        TracingHandler.h)

target_link_libraries(LinuxCaptureService PUBLIC
        ApiLoader
        ApiUtils
        CaptureService
        ProducerEventProcessor
        GrpcProtos
        Introspection
        LinuxTracing
        MemoryTracing
        OrbitBase
        OrbitVersion
        UserSpaceInstrumentation)

