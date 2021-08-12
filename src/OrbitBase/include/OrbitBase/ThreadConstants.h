// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_THREAD_CONSTANTS_H
#define ORBIT_BASE_THREAD_CONSTANTS_H

#include <cstdint>

namespace orbit_base {

// Note that -1 is reserved kInvalidThreadId in OrbitBase/ThreadUtils.h. Any non-multiple of 4 in
// the range [2^31+1, 2^32-1] won't clash with valid Linux or Windows thread ids.

// Represents a fake thread id to specify the set of all thread ids of the current process.
static constexpr uint32_t kAllProcessThreadsTid = -5;

// Represents a fake thread id to specify the set of all thread ids of all processes on the system.
static constexpr uint32_t kAllThreadsOfAllProcessesTid = -2;

// Represents a fake thread id to specify the set of all thread ids of all processes on the system
// that are NOT in the current process.
static constexpr uint32_t kNotTargetProcessTid = -3;
}  // namespace orbit_base

#endif  // ORBIT_BASE_THREAD_CONSTANTS_H
