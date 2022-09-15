# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(CodeViewer)
add_library(CodeViewer STATIC)

target_include_directories(CodeViewer PUBLIC include/)
target_link_libraries(CodeViewer PUBLIC CodeReport
                                        OrbitBase
                                        SyntaxHighlighter
                                        Qt5::Widgets)

target_sources(CodeViewer PUBLIC include/CodeViewer/Dialog.h
                                 include/CodeViewer/FontSizeInEm.h
                                 include/CodeViewer/OwningDialog.h
                                 include/CodeViewer/Viewer.h
                                 include/CodeViewer/PlaceHolderWidget.h)

target_sources(CodeViewer PRIVATE Dialog.cpp 
                                  Dialog.ui
                                  FontSizeInEm.cpp
                                  OwningDialog.cpp
                                  PlaceHolderWidget.cpp
                                  Viewer.cpp)

set_target_properties(CodeViewer PROPERTIES AUTOMOC ON AUTOUIC ON)

add_executable(CodeViewerTests)
target_sources(CodeViewerTests PRIVATE FontSizeInEmTest.cpp ViewerTest.cpp)
target_link_libraries(CodeViewerTests PRIVATE ClientFlags CodeViewer GTest::QtGuiMain)

if (WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen")
  register_test(CodeViewerTests PROPERTIES DISABLED TRUE)
endif()

if (NOT WIN32)
  register_test(CodeViewerTests PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()
