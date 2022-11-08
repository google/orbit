# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitClientGgpLib)

add_library(OrbitClientGgpLib STATIC)

target_include_directories(OrbitClientGgpLib PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitClientGgpLib PRIVATE
    ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitClientGgpLib PUBLIC
    include/OrbitClientGgp/ClientGgp.h
    include/OrbitClientGgp/ClientGgpOptions.h)

target_sources(OrbitClientGgpLib PRIVATE
    ClientGgp.cpp)

target_link_libraries(OrbitClientGgpLib PUBLIC
    CaptureClient
    ClientModel
    ClientServices
    GrpcProtos
    OrbitBase
    OrbitVersion
    SymbolProvider
    Symbols)

project(OrbitClientGgp)

add_executable(OrbitClientGgp main.cpp)

target_link_libraries(OrbitClientGgp PRIVATE 
    OrbitClientGgpLib
    absl::flags
    absl::flags_parse
    absl::flags_usage)
