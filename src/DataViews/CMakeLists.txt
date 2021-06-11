# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(DataViews STATIC)

target_sources(DataViews PRIVATE DataView.cpp)

target_sources(DataViews PUBLIC
        include/DataViews/AppInterface.h
        include/DataViews/DataView.h
        include/DataViews/DataViewType.h
        include/DataViews/PresetLoadState.h)

target_include_directories(DataViews PUBLIC include/)
target_compile_features(DataViews PUBLIC cxx_std_17)
target_link_libraries(DataViews PUBLIC
        ClientData
        ClientModel
        ClientProtos
        OrbitBase
        PresetFile
        CONAN_PKG::abseil)

add_executable(DataViewsTests)
target_compile_options(DataViewsTests PRIVATE ${STRICT_COMPILE_FLAGS})
target_sources(DataViewsTests PRIVATE DataViewTest.cpp)
target_link_libraries(DataViewsTests PRIVATE
        DataViews
        GTest::Main)

register_test(DataViewsTests)