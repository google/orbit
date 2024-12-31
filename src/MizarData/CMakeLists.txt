# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(MizarData STATIC)

target_sources(MizarData PUBLIC
         include/MizarData/BaselineAndComparison.h
         include/MizarData/GetCallstackSamplingIntervals.h
         include/MizarData/FrameTrack.h
         include/MizarData/FrameTrackManager.h
         include/MizarData/MizarData.h
         include/MizarData/MizarDataProvider.h
         include/MizarData/MizarPairedData.h)

target_include_directories(MizarData PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(MizarData PRIVATE
                BaselineAndComparison.cpp
                BaselineAndComparisonHelper.h
                DummyFunctionSymbolToKey.cpp
                DummyFunctionSymbolToKey.h
                GetCallstackSamplingIntervals.cpp
                MizarData.cpp)

target_link_libraries(
        MizarData
        PUBLIC CaptureClient
                MizarBase
                MizarStatistics
                OrbitPaths
                Statistics
                Symbols
        PRIVATE CaptureFile
                ClientData
                ClientSymbols
                OrbitBase)

add_executable(MizarDataTests)

target_sources(MizarDataTests PRIVATE
                BaselineAndComparisonTest.cpp
                DummyFunctionSymbolToKeyTest.cpp
                GetCallstackSamplingIntervalsTest.cpp
                FrameTrackManagerTest.cpp
                MizarDataTest.cpp
                MizarPairedDataTest.cpp)

target_link_libraries(MizarDataTests PRIVATE GrpcProtos
                                                MizarData
                                                GTest::gtest
                                                GTest_Main
                                                TestUtils)
register_test(MizarDataTests)
