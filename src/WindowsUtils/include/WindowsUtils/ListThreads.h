// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_LIST_THREADS_H_
#define WINDOWS_UTILS_LIST_THREADS_H_

#include <string>
#include <vector>

#include "OrbitBase/ThreadConstants.h"

namespace orbit_windows_utils {

struct Thread {
  uint32_t tid = orbit_base::kInvalidThreadId;
  uint32_t pid = orbit_base::kInvalidProcessId;
  std::string name;
};

// List all currently running threads of the process identified by "pid".
[[nodiscard]] std::vector<Thread> ListThreads(uint32_t pid);

// List all threads.
[[nodiscard]] std::vector<Thread> ListAllThreads();

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_LIST_THREADS_H_
