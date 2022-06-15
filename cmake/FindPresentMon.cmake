# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(PresentMon INTERFACE)

set(DIR third_party/PresentMon)
target_include_directories(PresentMon SYSTEM INTERFACE ${DIR})

add_library(PresentMon::PresentMon ALIAS PresentMon)
