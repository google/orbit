# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(Style OBJECT)

target_sources(Style PRIVATE Style.cpp)
target_sources(Style PUBLIC include/Style/Style.h)

target_include_directories(Style PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(Style PUBLIC Qt5::Widgets)