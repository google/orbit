# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ApiLoader)

add_library(ApiLoader STATIC)

target_compile_options(ApiLoader PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(ApiLoader PUBLIC cxx_std_17)

target_include_directories(ApiLoader PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(ApiLoader PUBLIC
        include/ApiLoader/EnableInTracee.h)

target_sources(ApiLoader PRIVATE
        EnableInTracee.cpp)

target_link_libraries(ApiLoader PUBLIC
        GrpcProtos
        ObjectUtils
        OrbitBase
        UserSpaceInstrumentation)
