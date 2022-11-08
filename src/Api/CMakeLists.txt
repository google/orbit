# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(Api)

add_library(Api SHARED)

if (WIN32)
# We don't use "orbit" to avoid a naming clash with Orbit.exe's pdb file.
set_target_properties(Api PROPERTIES OUTPUT_NAME "OrbitApi")
else()
set_target_properties(Api PROPERTIES OUTPUT_NAME "orbit")
endif()

target_sources(Api PRIVATE
        LockFreeApiEventProducer.h
        LockFreeApiEventProducer.cpp
        Orbit.cpp
        OrbitApiVersions.h)

target_link_libraries(Api PUBLIC
        ApiInterface
        ApiUtils
        CaptureEventProducer
        GrpcProtos
        OrbitBase
        ProducerSideChannel)

if (NOT WIN32)
install(TARGETS Api
        CONFIGURATIONS Release
        LIBRARY DESTINATION /opt/orbitprofiler)
endif()