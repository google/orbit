# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitClientModel)
add_library(OrbitClientModel STATIC)

target_compile_options(OrbitClientModel PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitClientModel PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitClientModel PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitClientModel PUBLIC
        include/OrbitClientModel/CaptureData.h
        include/OrbitClientModel/CaptureDeserializer.h
        include/OrbitClientModel/CaptureSerializer.h
        include/OrbitClientModel/SamplingDataPostProcessor.h)

target_sources(OrbitClientModel PRIVATE
        CaptureData.cpp
        CaptureDeserializer.cpp
        CaptureSerializer.cpp
        SamplingDataPostProcessor.cpp)

target_link_libraries(OrbitClientModel PUBLIC
        CaptureClient
        ClientProtos
        OrbitCore)

add_executable(OrbitClientModelTests)

target_compile_options(OrbitClientModelTests PRIVATE ${STRICT_COMPILE_FLAGS})


target_sources(OrbitClientModelTests PRIVATE
        CaptureDeserializerTest.cpp
        CaptureSerializationTestMatchers.h
        CaptureSerializerTest.cpp)

target_link_libraries(
        OrbitClientModelTests
        PRIVATE OrbitClientModel
        GTest::Main)

register_test(OrbitClientModelTests)

add_fuzzer(CaptureDeserializerLoadFuzzer CaptureDeserializerLoadFuzzer.cpp)
target_link_libraries(CaptureDeserializerLoadFuzzer
                      PRIVATE OrbitClientModel
                              CONAN_PKG::libprotobuf-mutator
                              GTest::GTest)