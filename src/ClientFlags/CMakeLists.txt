# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(ClientFlags OBJECT)

target_sources(ClientFlags PRIVATE ClientFlags.cpp)
target_sources(ClientFlags PUBLIC include/ClientFlags/ClientFlags.h)

target_include_directories(ClientFlags PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(ClientFlags PUBLIC absl::flags)
