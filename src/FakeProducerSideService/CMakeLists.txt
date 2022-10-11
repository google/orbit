# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(FakeProducerSideService INTERFACE)

target_sources(FakeProducerSideService INTERFACE
        include/FakeProducerSideService/FakeProducerSideService.h)

target_include_directories(FakeProducerSideService INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(FakeProducerSideService INTERFACE
        GrpcProtos
        grpc::grpc
        GTest::gtest)