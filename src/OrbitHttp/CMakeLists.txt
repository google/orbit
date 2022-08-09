# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitHttp)

add_library(OrbitHttp STATIC)

target_sources(OrbitHttp PRIVATE 
        HttpDownloadOperation.cpp
        HttpDownloadManager.cpp)
target_sources(OrbitHttp PUBLIC 
        include/OrbitHttp/HttpDownloadOperation.h
        include/OrbitHttp/HttpDownloadManager.h)

target_include_directories(OrbitHttp PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(OrbitHttp PUBLIC
        OrbitBase        
        Qt5::Core
        Qt5::Network)

set_target_properties(OrbitHttp PROPERTIES AUTOMOC ON)

add_executable(OrbitHttpTests)

target_sources(OrbitHttpTests PRIVATE 
        HttpDownloadManagerTest.cpp)

target_link_libraries(OrbitHttpTests PRIVATE
        OrbitBase 
        OrbitHttp      
        TestUtils 
        GTest::QtCoreMain
        QtUtils
        Qt5::Core)

register_test(OrbitHttpTests)