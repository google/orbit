// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_PROCESS_LIST_H_
#define WINDOWS_UTILS_PROCESS_LIST_H_

#include <optional>
#include <string>
#include <vector>

#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadConstants.h"

namespace orbit_windows_utils {

struct Process {
  uint32_t pid = orbit_base::kInvalidProcessId;
  std::string name;
  std::string full_path;
  std::string build_id;
  bool is_64_bit = true;
  double cpu_usage_percentage;
};

// Interface for listing all running processes and measuring their cpu usage. "Refresh()" must be
// called at least once to produce meaningful cpu usage values. This class is not thread safe.
class ProcessList {
 public:
  virtual ~ProcessList() = default;
  [[nodiscard]] virtual ErrorMessageOr<void> Refresh() = 0;
  [[nodiscard]] virtual std::vector<const Process*> GetProcesses() const = 0;
  [[nodiscard]] virtual std::optional<const Process*> GetProcessByPid(uint32_t pid) const = 0;

  [[nodiscard]] static std::unique_ptr<ProcessList> Create();
};

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_PROCESS_LIST_H_
