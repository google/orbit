# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(SourcePathsMappingUI STATIC)

target_sources(SourcePathsMappingUI PUBLIC include/SourcePathsMappingUI/AskUserForFile.h)
target_sources(SourcePathsMappingUI PRIVATE AskUserForFile.cpp)

target_include_directories(SourcePathsMappingUI PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(SourcePathsMappingUI PUBLIC SourcePathsMapping Qt5::Widgets)