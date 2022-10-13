# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(protobuf CONFIG QUIET)

if(TARGET protobuf::protobuf)
  return()
endif()

message(STATUS "Protobuf not found via find_package. Trying PkgConfig...")

find_package(PkgConfig REQUIRED)
pkg_check_modules(PROTOBUF REQUIRED IMPORTED_TARGET protobuf)
add_library(protobuf::protobuf ALIAS PkgConfig::PROTOBUF)
