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
        MizarData 
        PRIVATE 
                CaptureClient
                CaptureFile
                ClientData
                OrbitBase)
