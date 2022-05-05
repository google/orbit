# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ApiLoader)

add_library(ApiLoader STATIC)

target_include_directories(ApiLoader PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(ApiLoader PUBLIC
        include/ApiLoader/EnableInTracee.h)

if (WIN32)
target_sources(ApiLoader PRIVATE
        EnableInTraceeWindows.cpp)
else()
target_sources(ApiLoader PRIVATE
        EnableInTraceeLinux.cpp)
endif()

target_link_libraries(ApiLoader PUBLIC
        ApiUtils
        GrpcProtos
        ObjectUtils
        OrbitBase)

if (WIN32)
target_link_libraries(ApiLoader PUBLIC
        WindowsUtils)
else()
target_link_libraries(ApiLoader PUBLIC
        UserSpaceInstrumentation)
endif()
