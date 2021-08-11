# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(GrpcProtos)

add_library(GrpcProtos STATIC)

if(WIN32)
    target_compile_definitions(GrpcProtos PUBLIC -D_WIN32_WINNT=0x0700)
    target_compile_definitions(GrpcProtos PUBLIC -DNTDDI_VERSION=0x06030000)
endif()

target_include_directories(GrpcProtos PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(GrpcProtos PUBLIC
        include/GrpcProtos/Constants.h)

target_sources(GrpcProtos PRIVATE
        capture.proto
        code_block.proto
        module.proto
        process.proto
        producer_side_services.proto
        services.proto
        services_ggp.proto
        symbol.proto
        tracepoint.proto)

grpc_helper(GrpcProtos)
