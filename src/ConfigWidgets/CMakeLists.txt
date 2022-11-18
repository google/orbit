# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ConfigWidgets)
add_library(ConfigWidgets STATIC)

target_include_directories(ConfigWidgets PUBLIC include/)
target_include_directories(ConfigWidgets PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ConfigWidgets PUBLIC ClientData
                                           ClientFlags
                                           ClientSymbols
                                           OrbitBase
                                           SourcePathsMapping
                                           absl::flags
                                           absl::flat_hash_set
                                           absl::str_format
                                           Qt5::Core
                                           Qt5::Gui
                                           Qt5::Widgets)

target_sources(
  ConfigWidgets PUBLIC include/ConfigWidgets/PropertyConfigWidget.h
                       include/ConfigWidgets/SourcePathsMappingDialog.h
                       include/ConfigWidgets/StopSymbolDownloadDialog.h
                       include/ConfigWidgets/SymbolErrorDialog.h
                       include/ConfigWidgets/SymbolLocationsDialog.h)
target_sources(ConfigWidgets PRIVATE PropertyConfigWidget.cpp
                                     SourcePathsMappingDialog.cpp
                                     SourcePathsMappingDialog.ui
                                     StopSymbolDownloadDialog.cpp
                                     StopSymbolDownloadDialog.ui
                                     SymbolErrorDialog.cpp
                                     SymbolErrorDialog.ui
                                     SymbolLocationsDialog.cpp
                                     SymbolLocationsDialog.ui)

set_target_properties(ConfigWidgets PROPERTIES AUTOMOC ON)
set_target_properties(ConfigWidgets PROPERTIES AUTOUIC ON)

add_executable(ConfigWidgetsTests)

target_sources(ConfigWidgetsTests PRIVATE PropertyConfigWidgetTest.cpp
                                          StopSymbolDownloadDialogTest.cpp
                                          SymbolErrorDialogTest.cpp
                                          SymbolLocationsDialogTest.cpp)

target_link_libraries(ConfigWidgetsTests PRIVATE ConfigWidgets
                                                 TestUtils
                                                 GTest::QtGuiMain)

if(WIN32)
  # This is needed for the designated initializers. We don't enable this
  # in general to not break the build for old GCC and clang compilers for
  # no reason.
  target_compile_features(ConfigWidgetsTests PUBLIC cxx_std_20)
endif()

if (WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen")
  register_test(ConfigWidgetsTests PROPERTIES DISABLED TRUE)
endif()

if (NOT WIN32)
  register_test(ConfigWidgetsTests PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()