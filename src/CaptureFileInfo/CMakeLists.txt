# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(CaptureFileInfo)
add_library(CaptureFileInfo STATIC)

target_compile_options(CaptureFileInfo PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  CaptureFileInfo
  PUBLIC  include/CaptureFileInfo/CaptureFileInfo.h
          include/CaptureFileInfo/ItemModel.h
          include/CaptureFileInfo/Manager.h
          include/CaptureFileInfo/LoadCaptureWidget.h)

target_include_directories(CaptureFileInfo PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(
  CaptureFileInfo
  PRIVATE CaptureFileInfo.cpp
          ItemModel.cpp
          Manager.cpp
          LoadCaptureWidget.cpp
          LoadCaptureWidget.ui)


target_link_libraries(
  CaptureFileInfo
  PUBLIC  OrbitBase
          OrbitCore
          Qt5::Widgets)

set_target_properties(CaptureFileInfo PROPERTIES AUTOMOC ON)
set_target_properties(CaptureFileInfo PROPERTIES AUTOUIC ON)

add_executable(CaptureFileInfoTests)

target_compile_options(CaptureFileInfoTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  CaptureFileInfoTests 
  PRIVATE CaptureFileInfoTest.cpp
          ItemModelTest.cpp
          ManagerTest.cpp)

target_link_libraries(
  CaptureFileInfoTests
  PRIVATE CaptureFileInfo
          GTest::Main
          QtUtils
          Qt5::Test)

add_custom_command(TARGET CaptureFileInfoTests POST_BUILD 
          COMMAND ${CMAKE_COMMAND} -E remove_directory 
            $<TARGET_FILE_DIR:CaptureFileInfoTests>/testdata/CaptureFileInfo
          COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_SOURCE_DIR}/testdata/
            $<TARGET_FILE_DIR:CaptureFileInfoTests>/testdata/CaptureFileInfo)

register_test(CaptureFileInfoTests)