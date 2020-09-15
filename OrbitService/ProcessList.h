// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_PROCESS_LIST_
#define ORBIT_SERVICE_PROCESS_LIST_

#include <outcome.hpp>
#include <vector>

#include "OrbitBase/Result.h"
#include "Process.h"

namespace orbit_service {

class ProcessList {
 public:
  [[nodiscard]] ErrorMessageOr<void> Refresh();
  [[nodiscard]] std::vector<orbit_grpc_protos::ProcessInfo> GetProcesses() const {
    std::vector<orbit_grpc_protos::ProcessInfo> processes;
    processes.reserve(processes_.size());
    std::copy(processes_.begin(), processes_.end(), std::back_inserter(processes));

    return processes;
  }

 private:
  std::vector<Process> processes_;
  std::unordered_map<int32_t, Process*> processes_map_;
};

}  // namespace orbit_service

#endif  // ORBIT_SERVICE_PROCESS_LIST_
