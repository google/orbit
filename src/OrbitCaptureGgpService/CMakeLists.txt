# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitCaptureGgpService)

add_executable(OrbitCaptureGgpService main.cpp)

target_include_directories(OrbitCaptureGgpService PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitCaptureGgpService PRIVATE
        main.cpp
        OrbitCaptureGgpService.cpp
        OrbitCaptureGgpService.h
        OrbitCaptureGgpServiceImpl.cpp
        OrbitCaptureGgpServiceImpl.h)

target_link_libraries(OrbitCaptureGgpService PUBLIC
        GrpcProtos
        OrbitBase
        OrbitClientGgpLib
        absl::flags
        absl::flags_parse
        absl::flags_usage)

