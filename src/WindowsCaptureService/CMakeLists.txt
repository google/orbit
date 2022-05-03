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
        include/WindowsCaptureService/WindowsCaptureService.h)

target_sources(WindowsCaptureService PRIVATE
        WindowsCaptureService.cpp
        TracingHandler.cpp
        TracingHandler.h)

target_link_libraries(WindowsCaptureService PUBLIC
        ApiLoader
        ApiUtils
        CaptureServiceBase
        ProducerEventProcessor
        GrpcProtos
        Introspection
        OrbitBase
        OrbitVersion
        WindowsTracing)
