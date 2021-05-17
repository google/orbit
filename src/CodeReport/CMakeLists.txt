# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(CodeReport)
add_library(CodeReport INTERFACE)

target_compile_options(CodeReport INTERFACE ${STRICT_COMPILE_FLAGS})
target_include_directories(CodeReport INTERFACE include/)

target_sources(CodeReport INTERFACE include/CodeReport/CodeReport.h)