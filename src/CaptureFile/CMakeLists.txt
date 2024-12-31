# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(CaptureFile)
add_library(CaptureFile STATIC)

target_sources(
  CaptureFile
  PUBLIC include/CaptureFile/BufferOutputStream.h
         include/CaptureFile/CaptureFile.h
         include/CaptureFile/CaptureFileHelpers.h
         include/CaptureFile/CaptureFileOutputStream.h
         include/CaptureFile/CaptureFileSection.h
         include/CaptureFile/ProtoSectionInputStream.h)

target_sources(
  CaptureFile
  PRIVATE BufferOutputStream.cpp
          CaptureFileConstants.h
          CaptureFile.cpp
          CaptureFileHelpers.cpp
          CaptureFileOutputStream.cpp
          ProtoSectionInputStreamImpl.cpp
          ProtoSectionInputStreamImpl.h
          FileFragmentInputStream.cpp
          FileFragmentInputStream.h)

target_include_directories(CaptureFile PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(
  CaptureFile
  PUBLIC OrbitBase
         GrpcProtos
         ClientProtos
         protobuf::protobuf)

add_executable(CaptureFileTests)

target_sources(CaptureFileTests PRIVATE
  BufferOutputStreamTest.cpp
  CaptureFileHelpersTest.cpp
  CaptureFileOutputStreamTest.cpp
  CaptureFileTest.cpp
  FileFragmentInputStreamTest.cpp
)

target_link_libraries(
  CaptureFileTests
  PRIVATE CaptureFile
          TestUtils
          absl::base
          absl::synchronization
          GTest_Main
          )

register_test(CaptureFileTests)
