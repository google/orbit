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
        include/CaptureService/CaptureServiceBase.h
        include/CaptureService/CaptureServiceUtils.h
        include/CaptureService/CaptureStartStopListener.h
        include/CaptureService/ClientCaptureEventCollectorBuilder.h
        include/CaptureService/ClientCaptureEventCollectorBuilderImpl.h
        include/CaptureService/CommonProducerCaptureEventBuilders.h)

target_sources(CaptureService PRIVATE
        CaptureServiceBase.cpp
        CaptureServiceUtils.cpp
        CommonProducerCaptureEventBuilders.cpp)

target_link_libraries(CaptureService PUBLIC
        ProducerEventProcessor
        GrpcProtos
        Introspection
        ObjectUtils
        OrbitBase
        OrbitVersion)
