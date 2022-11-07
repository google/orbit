# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(gRPC CONFIG QUIET)

if(TARGET grpc::grpc)
  return()
endif()

message(STATUS "gRPC not found via find_package. Trying PkgConfig...")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED IMPORTED_TARGET grpc++_unsecure)
target_link_libraries(PkgConfig::GRPC INTERFACE protobuf::protobuf)
add_library(grpc::grpc ALIAS PkgConfig::GRPC)
