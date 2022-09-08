# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(Http)

add_library(Http STATIC)

target_sources(Http PRIVATE
        HttpDownloadOperation.cpp
        HttpDownloadOperation.h
        HttpDownloadManager.cpp)
target_sources(Http PUBLIC
        include/Http/DownloadManager.h
        include/Http/HttpDownloadManager.h
        include/Http/MockDownloadManager.h)

target_include_directories(Http PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(Http PUBLIC
        OrbitBase
        Qt5::Core
        Qt5::Network)

set_target_properties(Http PROPERTIES AUTOMOC ON)

add_executable(HttpTests)

target_sources(HttpTests PRIVATE
        HttpDownloadManagerTest.cpp)

target_link_libraries(HttpTests PRIVATE
        GTest::QtCoreMain
        Http
        OrbitBase
        QtUtils
        Qt5::Core
        TestUtils)

register_test(HttpTests)