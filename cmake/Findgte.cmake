# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(gte INTERFACE)

set(DIR third_party/gte)
target_include_directories(gte SYSTEM INTERFACE ${DIR})

add_library(gte::gte ALIAS gte)
