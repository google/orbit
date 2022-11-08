# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(FakeClient)

add_executable(OrbitFakeClient)

target_sources(OrbitFakeClient PRIVATE
        FakeCaptureEventProcessor.h
        Flags.h
        FakeClientMain.cpp
        GraphicsCaptureEventProcessor.h)

target_link_libraries(OrbitFakeClient PRIVATE
        ApiUtils
        CaptureClient
        ClientData
        ClientProtos
        GrpcProtos
        ModuleUtils
        ObjectUtils
        OrbitBase
        SymbolProvider
        absl::flags
        absl::flags_usage
        absl::flags_parse
        absl::flat_hash_map
        absl::strings
        absl::time)
