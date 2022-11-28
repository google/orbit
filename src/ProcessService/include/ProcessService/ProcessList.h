// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PROCESS_SERVICE_PROCESS_LIST_H_
#define PROCESS_SERVICE_PROCESS_LIST_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <sys/types.h>

#include <algorithm>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>

#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Result.h"
#include "Process.h"

namespace orbit_process_service_internal {

class ProcessList {
 public:
  [[nodiscard]] ErrorMessageOr<void> Refresh();
  [[nodiscard]] std::vector<orbit_grpc_protos::ProcessInfo> GetProcesses() const {
    std::vector<orbit_grpc_protos::ProcessInfo> processes;
    processes.reserve(processes_.size());

    std::transform(processes_.begin(), processes_.end(), std::back_inserter(processes),
                   [](const auto& pair) { return pair.second.process_info(); });
    return processes;
  }

  [[nodiscard]] std::optional<const Process*> GetProcessByPid(pid_t pid) const {
    const auto it = processes_.find(pid);
    if (it == processes_.end()) {
      return std::nullopt;
    }

    return &it->second;
  }

 private:
  absl::flat_hash_map<pid_t, Process> processes_;
};

}  // namespace orbit_process_service_internal

#endif  // PROCESS_SERVICE_PROCESS_LIST_H_
