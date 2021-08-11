# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(CodeViewerDemo)
add_executable(CodeViewerDemo main.cpp CodeExamples.h)

target_link_libraries(CodeViewerDemo PRIVATE CodeViewer
                                             Style
                                             SyntaxHighlighter
                                             Qt5::Gui)
