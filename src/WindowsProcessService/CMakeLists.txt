# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(WindowsProcessService)
add_library(WindowsProcessService STATIC)

target_include_directories(WindowsProcessService PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(WindowsProcessService PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(WindowsProcessService PUBLIC
        include/WindowsProcessService/ProcessServiceImpl.h)

target_sources(WindowsProcessService PRIVATE
        ProcessServiceImpl.cpp)

target_link_libraries(WindowsProcessService PUBLIC
        GrpcProtos
        ObjectUtils
        OrbitBase
        WindowsTracing
        WindowsUtils)

