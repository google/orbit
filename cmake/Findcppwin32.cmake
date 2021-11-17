# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(cppwin32 INTERFACE)

target_include_directories(cppwin32 SYSTEM INTERFACE 
    third_party/cppwin32/
    third_party/cppwin32/cppwin32/winmd/)

add_library(cppwin32::cppwin32 ALIAS cppwin32)
