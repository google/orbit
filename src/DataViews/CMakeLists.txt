# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(DataViews STATIC)

target_sources(DataViews PRIVATE
        CallstackDataView.cpp
        DataView.cpp
        DataViewUtils.h
        DataViewUtils.cpp
        FunctionsDataView.cpp
        LiveFunctionsDataView.cpp
        ModulesDataView.cpp
        PresetsDataView.cpp
        SamplingReportDataView.cpp
        TracepointsDataView.cpp)

target_sources(DataViews PUBLIC
        include/DataViews/AppInterface.h
        include/DataViews/CompareAscendingOrDescending.h
        include/DataViews/CallstackDataView.h
        include/DataViews/DataView.h
        include/DataViews/DataViewType.h
        include/DataViews/FunctionsDataView.h
        include/DataViews/LiveFunctionsDataView.h
        include/DataViews/LiveFunctionsInterface.h
        include/DataViews/ModulesDataView.h
        include/DataViews/PresetsDataView.h
        include/DataViews/PresetLoadState.h
        include/DataViews/SamplingReportDataView.h        
        include/DataViews/SamplingReportInterface.h
        include/DataViews/SymbolLoadingState.h
        include/DataViews/TracepointsDataView.h)

target_include_directories(DataViews PUBLIC include/)
target_link_libraries(DataViews PUBLIC
        ClientData
        ClientFlags
        ClientModel
        ClientProtos
        DisplayFormats
        OrbitBase
        PresetFile
        QtUtils
        Statistics
        SymbolProvider
        absl::flags
        absl::flat_hash_map
        absl::span
        absl::strings
        absl::str_format
        absl::time)

add_executable(DataViewsTests)
target_sources(DataViewsTests PRIVATE CallstackDataViewTest.cpp
                                      DataViewTest.cpp
                                      DataViewTestUtils.h
                                      DataViewTestUtils.cpp
                                      DataViewUtilsTest.cpp
                                      FunctionsDataViewTest.cpp
                                      LiveFunctionsDataViewTest.cpp
                                      MockAppInterface.h
                                      ModulesDataViewTest.cpp
                                      PresetsDataViewTest.cpp
                                      SamplingReportDataViewTest.cpp
                                      TracepointsDataViewTest.cpp)
target_link_libraries(DataViewsTests PRIVATE
        DataViews
        TestUtils
        GTest_Main)

register_test(DataViewsTests)
