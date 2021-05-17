# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(CodeReport)
add_library(CodeReport OBJECT)

target_compile_options(CodeReport PRIVATE ${STRICT_COMPILE_FLAGS})
target_include_directories(CodeReport PUBLIC include/)
target_link_libraries(CodeReport PUBLIC OrbitBase ClientData ElfUtils)

target_sources(CodeReport PUBLIC include/CodeReport/CodeReport.h
                                 include/CodeReport/SourceCodeReport.h)

target_sources(CodeReport PRIVATE SourceCodeReport.cpp)