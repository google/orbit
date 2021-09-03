# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(CrashServiceImpl)
add_library(CrashServiceImpl STATIC)

target_include_directories(CrashServiceImpl PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(CrashServiceImpl PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(CrashServiceImpl PUBLIC
        include/CrashServiceImpl/CrashServiceImpl.h)

target_sources(CrashServiceImpl PRIVATE
        CrashServiceImpl.cpp)

target_link_libraries(CrashServiceImpl PUBLIC
        GrpcProtos
        OrbitBase)
