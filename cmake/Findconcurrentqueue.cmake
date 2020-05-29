# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_library(concurrentqueue INTERFACE IMPORTED GLOBAL)
target_include_directories(concurrentqueue SYSTEM
                           INTERFACE third_party/concurrentqueue)
target_compile_features(concurrentqueue INTERFACE cxx_std_11)

add_library(concurrentqueue::concurrentqueue ALIAS concurrentqueue)
