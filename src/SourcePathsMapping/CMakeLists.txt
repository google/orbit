# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(SourcePathsMapping)
add_library(SourcePathsMapping STATIC)

target_compile_options(SourcePathsMapping PRIVATE ${STRICT_COMPILE_FLAGS})
target_include_directories(SourcePathsMapping PUBLIC include/)
target_link_libraries(SourcePathsMapping PUBLIC OrbitBase Qt5::Core)

target_sources(
  SourcePathsMapping PUBLIC include/SourcePathsMapping/Mapping.h
                            include/SourcePathsMapping/MappingManager.h)
target_sources(SourcePathsMapping PRIVATE Mapping.cpp MappingManager.cpp)

add_executable(SourcePathsMappingTests)
target_compile_options(SourcePathsMappingTests PRIVATE ${STRICT_COMPILE_FLAGS})
target_sources(SourcePathsMappingTests PRIVATE MappingTest.cpp MappingManagerTest.cpp)
target_link_libraries(SourcePathsMappingTests PRIVATE SourcePathsMapping
                                                      GTest::QtCoreMain)

register_test(SourcePathsMappingTests)
