# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(CodeReport)
add_library(CodeReport OBJECT)

target_include_directories(CodeReport PUBLIC include/)
target_link_libraries(CodeReport PUBLIC OrbitBase
                                        ClientData
                                        GrpcProtos
                                        ObjectUtils
                                        absl::flat_hash_map
                                        absl::strings
                                        capstone::capstone)

target_sources(CodeReport PUBLIC include/CodeReport/AnnotateDisassembly.h
                                 include/CodeReport/AnnotatingLine.h
                                 include/CodeReport/CodeReport.h
                                 include/CodeReport/Disassembler.h
                                 include/CodeReport/DisassemblyReport.h
                                 include/CodeReport/SourceCodeReport.h)

target_sources(CodeReport PRIVATE AnnotateDisassembly.cpp
                                  Disassembler.cpp
                                  DisassemblyReport.cpp
                                  SourceCodeReport.cpp)

add_executable(CodeReportTests)
target_sources(CodeReportTests PRIVATE AnnotateDisassemblyTest.cpp
                                       AssemblyTestLiterals.h
                                       DisassemblerTest.cpp
                                       DisassemblyReportTest.cpp
                                       SourceCodeReportTest.cpp)
target_link_libraries(CodeReportTests PRIVATE 
        CodeReport
        ModuleUtils
        SymbolProvider
        GTest_Main)
register_test(CodeReportTests)
