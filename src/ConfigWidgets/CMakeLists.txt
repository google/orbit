# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ConfigWidgets)
add_library(ConfigWidgets STATIC)

target_compile_options(ConfigWidgets PRIVATE ${STRICT_COMPILE_FLAGS})
target_include_directories(ConfigWidgets PUBLIC include/)
target_include_directories(ConfigWidgets PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ConfigWidgets PUBLIC OrbitBase
                                           SourcePathsMapping
                                           Qt5::Core
                                           Qt5::Gui
                                           Qt5::Widgets)

target_sources(
  ConfigWidgets PUBLIC include/ConfigWidgets/SourcePathsMappingDialog.h)
target_sources(ConfigWidgets PRIVATE SourcePathsMappingDialog.cpp
                                     SourcePathsMappingDialog.ui)

set_target_properties(ConfigWidgets PROPERTIES AUTOMOC ON)
set_target_properties(ConfigWidgets PROPERTIES AUTOUIC ON)