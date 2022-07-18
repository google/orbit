# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(MizarBase INTERFACE)

target_sources(MizarBase INTERFACE
  include/MizarBase/BaselineOrComparison.h
  include/MizarBase/SampledFunctionId.h
  include/MizarBase/Titles.h
  include/MizarBase/ThreadId.h)

target_include_directories(MizarBase INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(MizarBase
  INTERFACE OrbitBase
  Qt5::Core)