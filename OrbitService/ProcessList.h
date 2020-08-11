// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_PROCESS_LIST_
#define ORBIT_SERVICE_PROCESS_LIST_

#include <outcome.hpp>
#include <vector>

#include "OrbitBase/Result.h"
#include "process.pb.h"

namespace orbit_service {

class ProcessList {
 public:
  [[nodiscard]] ErrorMessageOr<void> Refresh();
  [[nodiscard]] const std::vector<orbit_grpc_protos::ProcessInfo>&
  GetProcesses() {
    return processes_;
  }

 private:
  std::vector<orbit_grpc_protos::ProcessInfo> processes_;
  std::unordered_map<int32_t, orbit_grpc_protos::ProcessInfo*> processes_map_;
};

}  // namespace orbit_service

#endif  // ORBIT_SERVICE_PROCESS_LIST_
