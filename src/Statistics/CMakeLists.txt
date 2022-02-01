# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(Statistics STATIC)

target_sources(Statistics PUBLIC
                include/Statistics/BinomialConfidenceInterval.h)

target_include_directories(Statistics PUBLIC
               ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(Statistics PRIVATE
                BinomialConfidenceInterval.cpp)

add_executable(WilsonBinomialConfidenceIntervalEstimatorTests)

target_sources(WilsonBinomialConfidenceIntervalEstimatorTests PRIVATE
          WilsonBinomialConfidenceIntervalEstimatorTests.cpp)

target_link_libraries(
  WilsonBinomialConfidenceIntervalEstimatorTests
  PRIVATE Statistics
          GTest::Main)

register_test(WilsonBinomialConfidenceIntervalEstimatorTests)