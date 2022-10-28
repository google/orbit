# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(imgui CONFIG QUIET)

if(TARGET imgui::imgui)
  return()
endif()

message(STATUS "IMGui not found via find_package. Trying PkgConfig...")

find_package(PkgConfig REQUIRED)
pkg_check_modules(IMGUI REQUIRED IMPORTED_TARGET imgui)
add_library(imgui::imgui ALIAS PkgConfig::IMGUI)
