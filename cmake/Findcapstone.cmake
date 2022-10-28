# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(capstone CONFIG QUIET)

if(TARGET capstone::capstone)
  return()
endif()

message(STATUS "Capstone not found via find_package. Trying via pkg-config...")

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAPSTONE REQUIRED IMPORTED_TARGET capstone)
add_library(capstone::capstone ALIAS PkgConfig::CAPSTONE)
