# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(FuzzingUtils INTERFACE)
target_sources(FuzzingUtils INTERFACE
        include/FuzzingUtils/ProtoFuzzer.h
        include/FuzzingUtils/Fuzzer.h)
target_include_directories(FuzzingUtils INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(FuzzingUtils INTERFACE OrbitBase libprotobuf-mutator::libprotobuf-mutator)
