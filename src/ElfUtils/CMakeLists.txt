# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ElfUtils)
add_library(ElfUtils STATIC)

target_compile_options(ElfUtils PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  ElfUtils
  PUBLIC include/ElfUtils/ElfFile.h
         include/ElfUtils/LinuxMap.h)

target_sources(
  ElfUtils
  PRIVATE ElfFile.cpp)

if (NOT WIN32)
target_sources(ElfUtils PRIVATE LinuxMap.cpp)
endif()

target_include_directories(ElfUtils PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(
  ElfUtils
  PUBLIC GrpcProtos
         OrbitBase
         CONAN_PKG::abseil
         CONAN_PKG::llvm_object
         CONAN_PKG::llvm_symbolize
         CONAN_PKG::Outcome)

add_executable(ElfUtilsTests)

target_sources(ElfUtilsTests PRIVATE
    ElfFileTest.cpp
)

if (NOT WIN32)
target_sources(ElfUtilsTests PRIVATE LinuxMapTest.cpp)
endif()

target_link_libraries(
  ElfUtilsTests
  PRIVATE ElfUtils
          GTest::Main
          CONAN_PKG::llvm_object
          CONAN_PKG::abseil)

add_custom_command(TARGET ElfUtilsTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/testdata
  $<TARGET_FILE_DIR:ElfUtilsTests>/testdata)

register_test(ElfUtilsTests)


add_fuzzer(ElfFileLoadSymbolsFuzzer ElfFileLoadSymbolsFuzzer.cpp)
target_link_libraries(ElfFileLoadSymbolsFuzzer ElfUtils)
