# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(ClientModel)
add_library(ClientModel STATIC)

target_compile_options(ClientModel PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(ClientModel PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(ClientModel PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(ClientModel PUBLIC
        include/ClientModel/CaptureData.h
        include/ClientModel/CaptureDeserializer.h
        include/ClientModel/CaptureSerializer.h
        include/ClientModel/SamplingDataPostProcessor.h)

target_sources(ClientModel PRIVATE
        CaptureData.cpp
        CaptureDeserializer.cpp
        CaptureSerializer.cpp
        SamplingDataPostProcessor.cpp)

target_link_libraries(ClientModel PUBLIC
        OrbitBase
        CaptureClient
        ClientProtos)

add_executable(ClientModelTests)

target_compile_options(ClientModelTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(ClientModelTests PRIVATE
        CaptureDeserializerTest.cpp
        CaptureSerializationTestMatchers.h
        CaptureSerializerTest.cpp
        SamplingDataPostProcessorTest.cpp)

target_link_libraries(ClientModelTests PRIVATE
        ClientModel
        GTest::Main)

register_test(ClientModelTests)

add_fuzzer(CaptureDeserializerLoadFuzzer CaptureDeserializerLoadFuzzer.cpp)
target_link_libraries(CaptureDeserializerLoadFuzzer
                      PRIVATE ClientModel
                              CONAN_PKG::libprotobuf-mutator
                              GTest::GTest)
