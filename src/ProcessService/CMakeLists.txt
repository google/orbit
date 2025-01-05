# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(ProcessService)
add_library(ProcessService STATIC)

target_include_directories(ProcessService PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(ProcessService PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(ProcessService PUBLIC
        include/ProcessService/CpuTime.h
        include/ProcessService/Process.h
        include/ProcessService/ProcessList.h
        include/ProcessService/ProcessServiceImpl.h)

target_sources(ProcessService PRIVATE
        Process.cpp
        ProcessList.cpp
        ProcessServiceImpl.cpp
        ProcessServiceUtils.cpp
        ProcessServiceUtils.h)

target_link_libraries(ProcessService PUBLIC
        GrpcProtos
        ModuleUtils
        ObjectUtils
        OrbitBase
        Symbols)

add_executable(ProcessServiceTests)

target_sources(ProcessServiceTests PRIVATE
        ProcessListTest.cpp
        ProcessTest.cpp
        ProcessServiceUtilsTest.cpp)

target_link_libraries(ProcessServiceTests PRIVATE
        TestUtils
        ProcessService
        GTest_Main)

register_test(ProcessServiceTests PROPERTIES TIMEOUT 10)

