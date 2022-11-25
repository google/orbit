# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(CaptureFileInfo)
add_library(CaptureFileInfo STATIC)

target_sources(
  CaptureFileInfo
  PUBLIC  include/CaptureFileInfo/CaptureFileInfo.h
          include/CaptureFileInfo/ItemModel.h
          include/CaptureFileInfo/Manager.h)

target_include_directories(CaptureFileInfo PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(
  CaptureFileInfo
  PRIVATE CaptureFileInfo.cpp
          ItemModel.cpp
          Manager.cpp)


target_link_libraries(
  CaptureFileInfo
  PUBLIC  DisplayFormats
          OrbitBase
          OrbitPaths
          Qt5::Widgets)

set_target_properties(CaptureFileInfo PROPERTIES AUTOMOC ON)
set_target_properties(CaptureFileInfo PROPERTIES AUTOUIC ON)

add_executable(CaptureFileInfoTests)

target_sources(
  CaptureFileInfoTests 
  PRIVATE CaptureFileInfoTest.cpp
          ItemModelTest.cpp
          ManagerTest.cpp)

target_link_libraries(
  CaptureFileInfoTests
  PRIVATE CaptureFileInfo
          ClientFlags
          GTest::QtGuiMain
          QtUtils
          TestUtils
          Qt5::Test)

if (WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen")
  register_test(CaptureFileInfoTests PROPERTIES DISABLED TRUE)
endif()

if (NOT WIN32)
  register_test(CaptureFileInfoTests PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()
