# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(MizarBase STATIC)

target_sources(MizarBase PUBLIC
  include/MizarBase/AbsoluteAddress.h
  include/MizarBase/BaselineOrComparison.h
  include/MizarBase/SampledFunctionId.h
  include/MizarBase/Time.h
  include/MizarBase/Titles.h
  include/MizarBase/ThreadId.h)

target_include_directories(MizarBase PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(MizarBase PRIVATE Titles.cpp)

target_link_libraries(MizarBase
  PUBLIC OrbitBase
  Qt5::Core)