# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitProtos)

add_library(OrbitProtos STATIC)
target_compile_options(OrbitProtos PRIVATE ${STRICT_COMPILE_FLAGS})
target_compile_definitions(OrbitProtos PRIVATE -D_WIN32_WINNT=0x0700)
target_compile_definitions(OrbitProtos PRIVATE -DNTDDI_VERSION=0x06030000)

target_sources(OrbitProtos PRIVATE
        capture.proto
        code_block.proto
        module.proto
        process.proto
        producer_side_services.proto
        services.proto
        services_ggp.proto
        symbol.proto
        tracepoint.proto)

grpc_helper(OrbitProtos)
