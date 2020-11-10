// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_TIMES_H_
#define ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_TIMES_H_

#include <chrono>

struct LayerTimes {
  std::chrono::steady_clock::time_point last_frame;
  std::chrono::steady_clock::time_point capture_started;
};

#endif  // ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_TIMES_H_