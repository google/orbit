# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitFlags)

add_library(OrbitFlags STATIC)

target_compile_options(OrbitFlags PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(OrbitFlags PUBLIC cxx_std_17)

target_include_directories(OrbitBase PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(OrbitFlags PUBLIC
        include/OrbitFlags/Declarations.h)

target_sources(OrbitFlags PRIVATE FlagDefinitions.cpp)

target_link_libraries(OrbitFlags PUBLIC
        CONAN_PKG::abseil)
