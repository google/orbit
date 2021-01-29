# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitAPI)

add_library(OrbitAPI SHARED)

set_target_properties(OrbitAPI PROPERTIES OUTPUT_NAME "orbit")

target_compile_options(OrbitAPI PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(OrbitAPI PUBLIC cxx_std_17)

target_include_directories(OrbitAPI PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitAPI PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitAPI PUBLIC
        include/OrbitAPI/EncodedEvent.h
        include/OrbitAPI/LockFreeApiEventProducer.h
        include/OrbitAPI/Orbit.h
        include/OrbitAPI/Stubs.h)

target_sources(OrbitAPI PRIVATE
        Orbit.cpp
        Stubs.cpp)

target_link_libraries(OrbitAPI PUBLIC
        GrpcProtos
        OrbitBase
        OrbitProducer
        ProducerSideChannel)
