// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef ORBIT_BASE_THREAD_CONSTANTS_H
#define ORBIT_BASE_THREAD_CONSTANTS_H

#include <cstdint>

namespace orbit_base {

// Represents a fake thread id to specify the set of all thread ids of the current process.
static constexpr int32_t kAllProcessThreadsTid = -1;

// Represents a fake thread id to specify the set of all thread ids of all processes on the system.
static constexpr int32_t kAllThreadsOfAllProcessesTid = -2;

// Represents a fake thread id to specify the set of all thread ids of all processes on the system
// that are NOT in the current process.
static constexpr int32_t kNotTargetProcessTid = -3;

}  // namespace orbit_base
#endif  // ORBIT_BASE_THREAD_CONSTANTS_H
