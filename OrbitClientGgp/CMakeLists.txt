# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitClientGgp)

add_executable(OrbitClientGgp main.cpp)

target_compile_options(OrbitClientGgp PRIVATE
    ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitClientGgp PRIVATE
    ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitClientGgp PRIVATE
    ClientGgp.cpp
    ClientGgp.h
    main.cpp)

target_link_libraries(OrbitClientGgp PUBLIC
    OrbitBase
    OrbitCaptureClient
    OrbitClientModel
    OrbitClientServices
    OrbitProtos)
