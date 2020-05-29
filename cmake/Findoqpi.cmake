# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(oqpi INTERFACE)

set(DIR third_party/oqpi)
target_include_directories(oqpi SYSTEM INTERFACE ${DIR}/include)

target_compile_definitions(oqpi INTERFACE WIN32_LEAN_AND_MEAN)
target_compile_definitions(oqpi INTERFACE VC_EXTRALEAN)

add_library(oqpi::oqpi ALIAS oqpi)
