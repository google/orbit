# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitClientData)
add_library(OrbitClientData STATIC)

target_compile_options(OrbitClientData PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitClientData PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitClientData PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitClientData PUBLIC
        include/OrbitClientData/FunctionInfoSet.h
        include/OrbitClientData/FunctionUtils.h
        include/OrbitClientData/ModuleData.h
        include/OrbitClientData/ProcessData.h)

target_sources(OrbitClientData PRIVATE
        FunctionUtils.cpp
        ModuleData.cpp
        ProcessData.cpp)

target_link_libraries(OrbitClientData PUBLIC 
        OrbitClientProtos
        OrbitBase
        OrbitProtos
        xxHash::xxHash)


add_executable(OrbitClientDataTests)
target_compile_options(OrbitClientDataTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitClientDataTests PRIVATE
        FunctionInfoSetTest.cpp
        ModuleDataTest.cpp
        ProcessDataTest.cpp)

target_link_libraries(OrbitClientDataTests PRIVATE 
        OrbitClientData
        GTest::Main)

register_test(OrbitClientDataTests)
