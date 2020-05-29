# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(DIR third_party/xxHash-r42)

add_library(xxHash OBJECT ${DIR}/xxhash.c)
target_include_directories(xxHash SYSTEM PUBLIC ${DIR})

add_library(xxHash::xxHash ALIAS xxHash)
