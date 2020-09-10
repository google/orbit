# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitClientModel)
add_library(OrbitClientModel STATIC)

target_compile_options(OrbitClientModel PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitClientModel PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitClientModel PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitClientModel PUBLIC
        include/OrbitClientModel/CaptureDeserializer.h
        include/OrbitClientModel/CaptureSerializer.h)

target_sources(OrbitClientModel PRIVATE
        CaptureDeserializer.cpp
        CaptureSerializer.cpp)

target_link_libraries(OrbitClientModel PUBLIC
        OrbitCaptureClient
        OrbitCore
        OrbitProtos)
