# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(ApiInterface INTERFACE)

target_sources(ApiInterface INTERFACE
        include/ApiInterface/Orbit.h)

target_include_directories(ApiInterface INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include)