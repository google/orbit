# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

set(REQUIRED_LLVM_LIBS LLVMDebugInfoCodeView
                        LLVMDebugInfoDWARF
                        LLVMDebugInfoPDB
                        LLVMObject
                        LLVMSymbolize)

#find_llvm_libs("${REQUIRED_LLVM_LIBS}")

project(ObjectUtils)
add_library(ObjectUtils STATIC)

target_sources(
  ObjectUtils
  PUBLIC include/ObjectUtils/CoffFile.h
         include/ObjectUtils/ElfFile.h
         include/ObjectUtils/ObjectFile.h
         include/ObjectUtils/PdbFile.h
         include/ObjectUtils/SymbolsFile.h
         include/ObjectUtils/WindowsBuildIdUtils.h)

target_sources(
  ObjectUtils
  PRIVATE
        CoffFile.cpp
        ElfFile.cpp
        PdbFile.cpp
        PdbFileLlvm.h
        PdbFileLlvm.cpp
        ObjectFile.cpp
        SymbolsFile.cpp
        WindowsBuildIdUtils.cpp)

if (WIN32)
target_sources(
  ObjectUtils
  PRIVATE PdbFileDia.h
          PdbFileDia.cpp
          PdbUtilsDia.cpp)
target_sources(
  ObjectUtils
  PUBLIC include/ObjectUtils/PdbUtilsDia.h)
endif()

target_include_directories(ObjectUtils PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(
  ObjectUtils
  PUBLIC GrpcProtos
         OrbitBase
         Introspection
         absl::base
         absl::flat_hash_map
         absl::flat_hash_set
         absl::memory
         absl::str_format
         absl::strings
         absl::synchronization
         absl::time
         absl::span
         llvm-core::llvm-core
         )

if (WIN32)
target_link_libraries(ObjectUtils PUBLIC DIASDK::DIASDK)

add_custom_command(TARGET ObjectUtils POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DIASDK_DLL}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/")
endif()

add_executable(ObjectUtilsTests)

target_sources(ObjectUtilsTests PRIVATE
        CoffFileTest.cpp
        ElfFileTest.cpp
        ObjectFileTest.cpp
        PdbFileTest.h
        PdbFileLlvmTest.cpp
        SymbolsFileTest.cpp
        WindowsBuildIdUtilsTest.cpp)

if (WIN32)
target_sources(ObjectUtilsTests PRIVATE
        PdbFileDiaTest.cpp)
endif()

target_link_libraries(
  ObjectUtilsTests
  PRIVATE ObjectUtils
          TestUtils
          GTest_Main)

register_test(ObjectUtilsTests)

