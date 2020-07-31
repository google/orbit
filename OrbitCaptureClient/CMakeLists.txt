# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitCaptureClient)
add_library(OrbitCaptureClient STATIC)

target_compile_options(OrbitCaptureClient PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  OrbitCaptureClient
  PUBLIC CaptureClient.h
         CaptureEventProcessor.h
         CaptureListener.h)

target_sources(
  OrbitCaptureClient
  PRIVATE CaptureClient.cpp
          CaptureEventProcessor.cpp)

target_include_directories(OrbitCaptureClient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitCaptureClient
  PUBLIC OrbitCore
         OrbitProtos)

