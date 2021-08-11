# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(SyntaxHighlighter)
add_library(SyntaxHighlighter STATIC)

target_include_directories(SyntaxHighlighter PUBLIC include/)
target_link_libraries(SyntaxHighlighter PUBLIC OrbitBase Qt5::Gui)
set_target_properties(SyntaxHighlighter PROPERTIES AUTOMOC ON)

target_sources(SyntaxHighlighter PUBLIC include/SyntaxHighlighter/Cpp.h
                                        include/SyntaxHighlighter/HighlightingMetadata.h
                                        include/SyntaxHighlighter/X86Assembly.h)

target_sources(SyntaxHighlighter PRIVATE Cpp.cpp
                                         X86Assembly.cpp)
