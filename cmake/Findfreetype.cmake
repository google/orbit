# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(freetype CONFIG QUIET)

if(TARGET freetype)
  return()
endif()

message(STATUS "Freetype not found via find_package. Trying pkg-config...")

find_package(PkgConfig REQUIRED)
pkg_check_modules(FREETYPE REQUIRED IMPORTED_TARGET freetype2)
add_library(freetype ALIAS PkgConfig::FREETYPE)
