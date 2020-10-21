# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitCaptureGgpClientLib)
add_library(OrbitCaptureGgpClientLib STATIC)

target_compile_options(OrbitCaptureGgpClientLib PRIVATE 
        ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitCaptureGgpClientLib PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitCaptureGgpClientLib PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitCaptureGgpClientLib PUBLIC
        include/OrbitCaptureGgpClient/OrbitCaptureGgpClient.h)

target_sources(OrbitCaptureGgpClientLib PRIVATE
        OrbitCaptureGgpClient.cpp)

target_link_libraries(OrbitCaptureGgpClientLib PUBLIC 
        OrbitCore
        OrbitProtos)

project(OrbitCaptureGgpClient)
add_executable(OrbitCaptureGgpClient main.cpp)

target_link_libraries(OrbitCaptureGgpClient PRIVATE 
    OrbitCaptureGgpClientLib)

target_compile_options(OrbitCaptureGgpClient PRIVATE 
    ${STRICT_COMPILE_FLAGS})

strip_symbols(OrbitCaptureGgpClient)
