# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(TracingInterface)

add_library(TracingInterface INTERFACE)

target_sources(TracingInterface INTERFACE
        include/TracingInterface/Tracer.h
        include/TracingInterface/TracerListener.h)

target_include_directories(TracingInterface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(TracingInterface INTERFACE
        GrpcProtos)
