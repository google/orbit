# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(stb INTERFACE)

set(DIR third_party/stb)
target_include_directories(stb SYSTEM INTERFACE ${DIR})

target_compile_definitions(stb INTERFACE WIN32_LEAN_AND_MEAN)
target_compile_definitions(stb INTERFACE VC_EXTRALEAN)

add_library(stb::stb ALIAS stb)
