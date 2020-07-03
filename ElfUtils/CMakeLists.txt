# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ElfUtils)
add_library(ElfUtils STATIC)

target_compile_options(ElfUtils PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  ElfUtils
  PUBLIC include/ElfUtils/ElfFile.h)

target_sources(
  ElfUtils
  PRIVATE ElfFile.cpp)

target_include_directories(ElfUtils PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(
  ElfUtils
  PUBLIC OrbitBase
         OrbitProtos
         abseil::abseil
         llvm_object::llvm_object
         Outcome::Outcome)

add_executable(ElfUtilsTests)

target_sources(ElfUtilsTests PRIVATE
    ElfFileTest.cpp
)

target_link_libraries(
  ElfUtilsTests
  PRIVATE ElfUtils
          OrbitCore # TODO: This is needed for GetExecutablePath - consider moving it to OrbitBase
          GTest::Main
          GMock::GMock
          llvm_object::llvm_object
          abseil::abseil)

add_custom_command(TARGET ElfUtilsTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/testdata
  $<TARGET_FILE_DIR:ElfUtilsTests>/testdata)

register_test(ElfUtilsTests)
