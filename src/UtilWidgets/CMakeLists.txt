# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(UtilWidgets)
add_library(UtilWidgets STATIC)

target_sources(
  UtilWidgets
  PUBLIC  include/UtilWidgets/NoticeWidget.h)

target_include_directories(UtilWidgets PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(
  UtilWidgets
  PRIVATE NoticeWidget.cpp
          NoticeWidget.ui)

target_link_libraries(
  UtilWidgets
  PUBLIC  QtUtils
          absl::str_format
          Qt5::Widgets)

set_target_properties(UtilWidgets PROPERTIES AUTOMOC ON)
set_target_properties(UtilWidgets PROPERTIES AUTOUIC ON)
