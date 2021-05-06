# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(ClientData)
add_library(ClientData STATIC)

target_compile_options(ClientData PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(ClientData PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(ClientData PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(ClientData PUBLIC
        include/ClientData/Callstack.h
        include/ClientData/CallstackData.h
        include/ClientData/CallstackTypes.h
        include/ClientData/FunctionInfoSet.h
        include/ClientData/FunctionUtils.h
        include/ClientData/ModuleData.h
        include/ClientData/ModuleManager.h
        include/ClientData/PostProcessedSamplingData.h
        include/ClientData/ProcessData.h
        include/ClientData/TracepointCustom.h
        include/ClientData/TracepointData.h
        include/ClientData/UserDefinedCaptureData.h)

target_sources(ClientData PRIVATE
        CallstackData.cpp
        FunctionUtils.cpp
        ModuleData.cpp
        ModuleManager.cpp
        PostProcessedSamplingData.cpp
        ProcessData.cpp
        TracepointData.cpp
        UserDefinedCaptureData.cpp)

target_link_libraries(ClientData PUBLIC
        ClientProtos
        GrpcProtos
        OrbitBase
        xxHash::xxHash)


add_executable(ClientDataTests)
target_compile_options(ClientDataTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(ClientDataTests PRIVATE
        CallstackDataTest.cpp
        FunctionInfoSetTest.cpp
        ModuleDataTest.cpp
        ModuleManagerTest.cpp
        ProcessDataTest.cpp
        TracepointDataTest.cpp
        UserDefinedCaptureDataTest.cpp)

target_link_libraries(ClientDataTests PRIVATE
        ClientData
        GTest::Main)

register_test(ClientDataTests)
