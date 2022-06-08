# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ConfigWidgets)
add_library(ConfigWidgets STATIC)

target_include_directories(ConfigWidgets PUBLIC include/)
target_include_directories(ConfigWidgets PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ConfigWidgets PUBLIC ClientFlags
                                           OrbitBase
                                           SourcePathsMapping
                                           CONAN_PKG::abseil
                                           Qt5::Core
                                           Qt5::Gui
                                           Qt5::Widgets)

target_sources(
  ConfigWidgets PUBLIC include/ConfigWidgets/SourcePathsMappingDialog.h
                       include/ConfigWidgets/SymbolErrorDialog.h
                       include/ConfigWidgets/SymbolLocationsDialog.h)
target_sources(ConfigWidgets PRIVATE SourcePathsMappingDialog.cpp
                                     SourcePathsMappingDialog.ui
                                     SymbolErrorDialog.cpp
                                     SymbolErrorDialog.ui
                                     SymbolLocationsDialog.cpp
                                     SymbolLocationsDialog.ui)

set_target_properties(ConfigWidgets PROPERTIES AUTOMOC ON)
set_target_properties(ConfigWidgets PROPERTIES AUTOUIC ON)

add_executable(ConfigWidgetsTests)

target_sources(ConfigWidgetsTests PRIVATE SymbolErrorDialogTest.cpp
                                          SymbolLocationsDialogTest.cpp)

target_link_libraries(ConfigWidgetsTests PRIVATE ConfigWidgets
                                                 TestUtils
                                                 GTest::QtGuiMain)

if (WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen")
  register_test(ConfigWidgetsTests PROPERTIES DISABLED TRUE)
endif()

if (NOT WIN32)
  register_test(ConfigWidgetsTests PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()