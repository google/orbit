# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(FakeClient)

add_executable(OrbitFakeClient)

target_compile_options(OrbitFakeClient PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitFakeClient PRIVATE
        FakeCaptureEventProcessor.h
        FakeClientMain.cpp)

target_link_libraries(OrbitFakeClient PRIVATE
        CONAN_PKG::abseil
        CaptureClient
        ClientData
        ClientProtos
        ElfUtils
        GrpcProtos
        OrbitBase)

strip_symbols(OrbitFakeClient)
