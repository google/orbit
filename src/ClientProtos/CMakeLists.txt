# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ClientProtos)

add_library(ClientProtos STATIC)

protobuf_generate(TARGET ClientProtos PROTOS
        capture_data.proto
        preset.proto)

target_include_directories(ClientProtos PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(ClientProtos PUBLIC CONAN_PKG::protobuf)
