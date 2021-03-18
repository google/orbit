# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(ApiInterface INTERFACE)

target_sources(ApiInterface INTERFACE
        include/Api/EncodedEvent.h
        include/Api/LockFreeApiEventProducer.h
        include/Api/Orbit.h)

target_include_directories(ApiInterface INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include)

if(NOT WIN32)
project(Api)

add_library(Api SHARED)

set_target_properties(Api PROPERTIES OUTPUT_NAME "orbit")

target_compile_options(Api PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(Api PUBLIC cxx_std_17)

target_sources(Api PRIVATE
        Orbit.cpp)

target_link_libraries(Api PUBLIC
        ApiInterface
        GrpcProtos
        OrbitBase
        OrbitProducer
        ProducerSideChannel)
endif()


