# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(MizarStatistics INTERFACE)

target_sources(MizarStatistics INTERFACE
  include/MizarStatistics/ActiveFunctionTimePerFrameComparator.h)

target_include_directories(MizarStatistics INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(MizarStatistics INTERFACE OrbitBase
  MizarBase
  Statistics)

add_executable(MizarStatisticsTests)

target_sources(MizarStatisticsTests PRIVATE
  ActiveFunctionTimePerFrameComparatorTest.cpp)

target_link_libraries(MizarStatisticsTests PRIVATE GrpcProtos
  MizarStatistics
  GTest::gtest
  GTest_Main)

register_test(MizarStatisticsTests)