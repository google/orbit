# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(CrashService)
add_library(CrashService STATIC)

target_include_directories(CrashService PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(CrashService PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(CrashService PUBLIC
        include/CrashService/CrashServiceImpl.h)

target_sources(CrashService PRIVATE
        CrashServiceImpl.cpp)

target_link_libraries(CrashService PUBLIC
        GrpcProtos
        OrbitBase)
