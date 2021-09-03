# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(FramePointerValidatorService)
add_library(FramePointerValidatorService STATIC)

target_include_directories(FramePointerValidatorService PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(FramePointerValidatorService PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(FramePointerValidatorService PUBLIC
        include/FramePointerValidatorService/FramePointerValidatorServiceImpl.h)

target_sources(FramePointerValidatorService PRIVATE
        FramePointerValidatorServiceImpl.cpp)

target_link_libraries(FramePointerValidatorService PUBLIC
        FramePointerValidator
        GrpcProtos
        ObjectUtils
        OrbitBase)
