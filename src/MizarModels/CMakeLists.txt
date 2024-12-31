# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(MizarModels INTERFACE)

target_sources(MizarModels INTERFACE
  include/MizarModels/FrameTrackListModel.h
  include/MizarModels/SamplingWithFrameTrackReportModel.h)

target_include_directories(MizarModels INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(MizarModels INTERFACE
  MizarBase
  MizarData
  OrbitBase
  Qt5::Core)

add_executable(MizarModelsTests)

target_sources(MizarModelsTests PRIVATE
  FrameTrackListModelTest.cpp
  SamplingWithFrameTrackReportModelTest.cpp)

target_link_libraries(MizarModelsTests PRIVATE
  MizarModels
  TestUtils
  GTest_Main
  gtest
  gmock
  Qt5::Test
)

register_test(MizarModelsTests)