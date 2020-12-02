# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(MetricsUploader)

add_library(MetricsUploader STATIC)

target_compile_options(MetricsUploader PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(MetricsUploader PUBLIC cxx_std_17)

target_include_directories(MetricsUploader PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
                                           PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_sources(MetricsUploader PUBLIC include/MetricsUploader/MetricsUploader.h
                                      include/MetricsUploader/Result.h
                               PRIVATE MetricsUploader.cpp
                                       Result.cpp)

if (WIN32)
target_sources(MetricsUploader PRIVATE MetricsUploaderWindows.cpp)
else()
target_sources(MetricsUploader PRIVATE MetricsUploaderLinux.cpp)
endif()

target_link_libraries(MetricsUploader PUBLIC OrbitBase
                                             OrbitVersion
                                             CONAN_PKG::abseil)

# generate protos for communication
protobuf_generate(TARGET MetricsUploader PROTOS proto/orbit_log_event.proto)
target_include_directories(MetricsUploader PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(MetricsUploader PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/proto)
target_link_libraries(MetricsUploader PUBLIC CONAN_PKG::protobuf)
