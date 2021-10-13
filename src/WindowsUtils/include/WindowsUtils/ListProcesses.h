// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_LIST_PROCESSES_H_
#define WINDOWS_UTILS_LIST_PROCESSES_H_

#include <string>
#include <vector>

#include "OrbitBase/ThreadConstants.h"

namespace orbit_windows_utils {

struct Process {
  uint32_t pid = orbit_base::kInvalidProcessId;
  std::string name;
  std::string full_path;
  std::string build_id;
  bool is_64_bit = true;
};

// List all currently running processes.
[[nodiscard]] std::vector<Process> ListProcesses();

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_LIST_PROCESSES_H_
