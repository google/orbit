# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(ProducerSideChannel)

add_library(ProducerSideChannel INTERFACE)

target_sources(ProducerSideChannel INTERFACE
        include/ProducerSideChannel/ProducerSideChannel.h)

target_include_directories(ProducerSideChannel INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(ProducerSideChannel INTERFACE
        absl::str_format
        grpc::grpc)
