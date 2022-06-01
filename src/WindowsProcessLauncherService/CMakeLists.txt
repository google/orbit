# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(WindowsProcessLauncherService)
add_library(WindowsProcessLauncherService STATIC)

target_include_directories(WindowsProcessLauncherService PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(WindowsProcessLauncherService PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(WindowsProcessLauncherService PUBLIC
        include/WindowsProcessLauncherService/ProcessLauncherServiceImpl.h)

target_sources(WindowsProcessLauncherService PRIVATE
        ProcessLauncherServiceImpl.cpp)

target_link_libraries(WindowsProcessLauncherService PUBLIC
        GrpcProtos
        OrbitBase
        WindowsTracing
        WindowsUtils)
