# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(ObjectUtils)
add_library(ObjectUtils STATIC)

target_sources(
  ObjectUtils
  PUBLIC include/ObjectUtils/Address.h
         include/ObjectUtils/CoffFile.h
         include/ObjectUtils/ElfFile.h
         include/ObjectUtils/PdbFile.h
         include/ObjectUtils/LinuxMap.h
         include/ObjectUtils/SymbolsFile.h
         include/ObjectUtils/WindowsBuildIdUtils.h)

target_sources(
  ObjectUtils
  PRIVATE
        Address.cpp
        CoffFile.cpp
        ElfFile.cpp
        PdbFile.cpp
        PdbFileLlvm.h
        PdbFileLlvm.cpp
        ObjectFile.cpp
        SymbolsFile.cpp
        WindowsBuildIdUtils.cpp)

if (NOT WIN32)
target_sources(ObjectUtils PRIVATE LinuxMap.cpp)
else()
target_sources(
  ObjectUtils
  PRIVATE PdbFileDia.h
          PdbFileDia.cpp)
endif()

target_include_directories(ObjectUtils PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(
  ObjectUtils
  PUBLIC GrpcProtos
         OrbitBase
         CONAN_PKG::abseil
         CONAN_PKG::llvm-core)

if (WIN32)
target_link_libraries(ObjectUtils PUBLIC DIASDK::DIASDK)

add_custom_command(TARGET ObjectUtils POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${DIASDK_DLL} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

add_executable(ObjectUtilsTests)

target_sources(ObjectUtilsTests PRIVATE
        AddressTest.cpp
        CoffFileTest.cpp
        ElfFileTest.cpp
        ObjectFileTest.cpp
        PdbFileTest.h
        PdbFileLlvmTest.cpp
        SymbolsFileTest.cpp
        WindowsBuildIdUtilsTest.cpp
)

if (NOT WIN32)
target_sources(ObjectUtilsTests PRIVATE LinuxMapTest.cpp)
else()
target_sources(ObjectUtilsTests PRIVATE PdbFileDiaTest.cpp)
endif()

target_link_libraries(
  ObjectUtilsTests
  PRIVATE ObjectUtils
          TestUtils
          GTest::Main
          CONAN_PKG::llvm-core
          CONAN_PKG::abseil)

register_test(ObjectUtilsTests)

add_fuzzer(ElfFileLoadSymbolsFuzzer ElfFileLoadSymbolsFuzzer.cpp)
target_link_libraries(ElfFileLoadSymbolsFuzzer FuzzingUtils ObjectUtils)
