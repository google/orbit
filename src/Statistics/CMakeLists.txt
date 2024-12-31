# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(Statistics STATIC)

target_sources(Statistics PUBLIC
                include/Statistics/BinomialConfidenceInterval.h
                include/Statistics/DataSet.h
                include/Statistics/Gaussian.h
                include/Statistics/Histogram.h
                include/Statistics/MultiplicityCorrection.h
                include/Statistics/StatisticsUtils.h)

target_include_directories(Statistics PUBLIC
               ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(Statistics PRIVATE
                BinomialConfidenceInterval.cpp                
                DataSet.cpp
                Histogram.cpp
                HistogramUtils.h
                HistogramUtils.cpp)

target_link_libraries(Statistics PRIVATE OrbitBase)

add_executable(StatisticsTests)

target_sources(StatisticsTests PRIVATE
          GaussianTest.cpp
          HistogramTest.cpp
          MultiplicityCorrectionTest.cpp
          StatisticsUtilTest.cpp
          WilsonBinomialConfidenceIntervalEstimatorTest.cpp)

target_link_libraries(
  StatisticsTests
  PRIVATE Statistics
          GTest_Main
          TestUtils)

register_test(StatisticsTests)
