# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(Mizar CXX)

add_executable(Mizar)

target_sources(
        Mizar
        PRIVATE Mizar.cpp
        MizarMainWindow.h
        MizarMainWindow.cpp
        MizarMainWindow.ui
        SamplingWithFrameTrackInputWidget.h
        SamplingWithFrameTrackInputWidget.cpp
        SamplingWithFrameTrackInputWidget.ui
        SamplingWithFrameTrackWidget.h
        SamplingWithFrameTrackWidget.cpp
        SamplingWithFrameTrackWidget.ui)

target_include_directories(Mizar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Mizar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Mizar
        PUBLIC QtUtils
        Qt5::Widgets
        Qt5::Core
        PRIVATE CaptureClient
        CaptureFile
        ClientData
        MizarBase
        MizarData
        OrbitBase
        Threads::Threads
)

set_target_properties(Mizar PROPERTIES AUTOMOC ON)
set_target_properties(Mizar PROPERTIES AUTOUIC ON)
