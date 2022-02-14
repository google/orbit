# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(CaptureServiceBase)
add_library(CaptureServiceBase STATIC)

target_include_directories(CaptureServiceBase PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(CaptureServiceBase PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(CaptureServiceBase PUBLIC
        include/CaptureServiceBase/CaptureServiceBase.h
        include/CaptureServiceBase/CaptureStartStopListener.h
        include/CaptureServiceBase/ClientCaptureEventCollectorManager.h
        include/CaptureServiceBase/CloudCollectorStartStopCaptureRequestWaiter.h
        include/CaptureServiceBase/CommonProducerCaptureEventBuilders.h
        include/CaptureServiceBase/GrpcClientCaptureEventCollectorManager.h
        include/CaptureServiceBase/GrpcStartStopCaptureRequestWaiter.h
        include/CaptureServiceBase/StartStopCaptureRequestWaiter.h
        include/CaptureServiceBase/UploaderClientCaptureEventCollectorManager.h)

target_sources(CaptureServiceBase PRIVATE
        CaptureServiceBase.cpp
        CloudCollectorStartStopCaptureRequestWaiter.cpp
        CommonProducerCaptureEventBuilders.cpp
        GrpcClientCaptureEventCollectorManager.cpp
        GrpcStartStopCaptureRequestWaiter.cpp
        UploaderClientCaptureEventCollectorManager.cpp)

target_link_libraries(CaptureServiceBase PUBLIC
        CaptureUploader
        ProducerEventProcessor
        GrpcProtos
        Introspection
        ObjectUtils
        OrbitBase
        OrbitVersion)
