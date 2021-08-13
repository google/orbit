# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(Api)

add_library(Api SHARED)

set_target_properties(Api PROPERTIES OUTPUT_NAME "orbit")

target_sources(Api PRIVATE
        LockFreeApiEventProducer.h
        LockFreeApiEventProducer.cpp
        Orbit.cpp
        OrbitV0.h)

target_link_libraries(Api PUBLIC
        ApiInterface
        ApiUtils
        CaptureEventProducer
        GrpcProtos
        OrbitBase
        ProducerSideChannel)

strip_symbols(Api)
