# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitCaptureClient)
add_library(OrbitCaptureClient STATIC)

target_compile_options(OrbitCaptureClient PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitCaptureClient PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitCaptureClient PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitCaptureClient PUBLIC 
        include/OrbitCaptureClient/CaptureClient.h
        include/OrbitCaptureClient/CaptureListener.h
        include/OrbitCaptureClient/CaptureEventProcessor.h)

target_sources(OrbitCaptureClient PRIVATE 
        CaptureClient.cpp
        CaptureEventProcessor.cpp)

target_link_libraries(OrbitCaptureClient PUBLIC 
        OrbitCore
        OrbitProtos)

