# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(Mizar CXX)

add_executable(Mizar)

target_sources(
        Mizar
        PRIVATE Mizar.cpp
)

target_include_directories(Mizar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Mizar
        PRIVATE QtUtils
        Qt5::Widgets
        Qt5::Core
        CaptureClient
        CaptureFile
        ClientData
        MizarBase
        MizarData
        MizarWidgets
        OrbitBase
        Threads::Threads
        absl::flags
        absl::flags_parse
)
