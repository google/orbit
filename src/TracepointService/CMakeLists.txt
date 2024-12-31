# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(TracepointService)
add_library(TracepointService STATIC)

target_include_directories(TracepointService PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(TracepointService PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(TracepointService PUBLIC
        include/TracepointService/TracepointServiceImpl.h)

target_sources(TracepointService PRIVATE
        ReadTracepoints.cpp
        ReadTracepoints.h
        TracepointServiceImpl.cpp)

target_link_libraries(TracepointService PUBLIC
        GrpcProtos
        OrbitBase)

add_executable(TracepointServiceTests)

target_sources(TracepointServiceTests PRIVATE
        ReadTracepointsTest.cpp)

target_link_libraries(TracepointServiceTests PRIVATE
        TestUtils
        TracepointService
        GTest_Main)

register_test(TracepointServiceTests)
