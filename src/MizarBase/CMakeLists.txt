# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(MizarBase STATIC)

target_sources(MizarBase PUBLIC
  include/MizarBase/BaselineOrComparison.h)

target_include_directories(MizarBase PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(MizarBase
  PUBLIC OrbitBase)