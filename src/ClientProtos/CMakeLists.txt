# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ClientProtos)

add_library(ClientProtos STATIC)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/protos/ClientProtos)
protobuf_generate(TARGET ClientProtos PROTOS
        capture_data.proto
        preset.proto
        user_defined_capture_info.proto
        PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/protos/ClientProtos/)

target_include_directories(ClientProtos PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/protos)

target_link_libraries(ClientProtos PUBLIC protobuf::protobuf)
