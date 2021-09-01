# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(krabsetw INTERFACE)

set(DIR third_party/krabsetw)
target_include_directories(krabsetw SYSTEM INTERFACE ${DIR})

add_library(krabsetw::krabsetw ALIAS krabsetw)
