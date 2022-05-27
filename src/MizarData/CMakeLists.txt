# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(MizarData STATIC)

target_sources(MizarData PUBLIC 
         include/MizarData/MizarData.h
         include/MizarData/MizarDataProvider.h)

target_include_directories(MizarData PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include) 

target_sources(MizarData PRIVATE
                MizarData.cpp)


target_link_libraries(
        MizarData PUBLIC CaptureClient
        PRIVATE 
                CaptureFile
                ClientData
                OrbitBase)

add_executable(MizarDataTests)

target_sources(MizarDataTests PRIVATE MizarDataTest.cpp)
target_link_libraries(MizarDataTests PRIVATE GrpcProtos
                                                MizarData
                                                GTest::GTest
                                                GTest::Main)
register_test(MizarDataTests)
