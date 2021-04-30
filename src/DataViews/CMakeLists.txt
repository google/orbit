# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(DataViews INTERFACE)

target_sources(DataViews INTERFACE
        include/DataViews/AppInterface.h
        include/DataViews/PresetLoadState.h)

target_include_directories(DataViews INTERFACE include/)
target_compile_features(DataViews INTERFACE cxx_std_17)
target_link_libraries(DataViews INTERFACE
        ClientData
        ClientModel
        ClientProtos
        OrbitBase
        CONAN_PKG::abseil)
