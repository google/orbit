# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(MizarWidgets STATIC)

target_sources(MizarWidgets PUBLIC
  include/MizarWidgets/MizarMainWindow.h
  include/MizarWidgets/SamplingWithFrameTrackInputWidget.h
  include/MizarWidgets/SamplingWithFrameTrackOutputWidget.h
  include/MizarWidgets/SamplingWithFrameTrackWidget.h
  include/MizarWidgets/SamplingWithFrameTrackReportConfigValidator.h)

target_include_directories(MizarWidgets PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(MizarWidgets PRIVATE
  MizarMainWindow.ui
  MizarMainWindow.cpp
  SamplingWithFrameTrackInputWidget.ui
  SamplingWithFrameTrackInputWidget.cpp
  SamplingWithFrameTrackOutputWidget.ui
  SamplingWithFrameTrackOutputWidget.cpp
  SamplingWithFrameTrackWidget.ui
  SamplingWithFrameTrackWidget.cpp)

target_link_libraries(MizarWidgets
  PUBLIC MizarData
  MizarModels
  QtUtils
  Qt5::Widgets
  Qt5::Core)

set_target_properties(MizarWidgets PROPERTIES AUTOMOC ON)
set_target_properties(MizarWidgets PROPERTIES AUTOUIC ON)

add_executable(MizarWidgetsTests)

target_sources(MizarWidgetsTests PRIVATE
  SamplingWithFrameTrackInputWidgetTest.cpp
  SamplingWithFrameTrackWidgetTest.cpp
  SamplingWithFrameTrackReportConfigValidatorTest.cpp
)

target_link_libraries(MizarWidgetsTests PRIVATE
  MizarWidgets
  TestUtils
  GTest::QtGuiMain
  Qt5::Test
)

if(WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen")
  register_test(MizarWidgetsTests PROPERTIES DISABLED TRUE)
endif()

if(NOT WIN32)
  register_test(MizarWidgetsTests PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()

iwyu_add_dependency(MizarWidgets)