# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitClientProtos)

add_library(OrbitClientProtos STATIC)
protobuf_generate(TARGET OrbitClientProtos PROTOS capture_data.proto)
target_include_directories(OrbitClientProtos PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
