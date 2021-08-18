// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_PROCESS_H_
#define ORBIT_SERVICE_PROCESS_H_

#include <sys/types.h>

#include "OrbitBase/Result.h"
#include "ServiceUtils.h"
#include "process.pb.h"

namespace orbit_service {

class Process {
 public:
  void UpdateCpuUsage(utils::Jiffies process_cpu_time, utils::TotalCpuTime total_cpu_time);

  // Creates a `Process` by reading details from the `/proc` filesystem.
  // This might fail due to a non existing pid or due to permission problems.
  static ErrorMessageOr<Process> FromPid(pid_t pid);

  // NOLINTNEXTLINE
  [[nodiscard]] const orbit_grpc_protos::ProcessInfo& process_info() const { return process_info_; }

 private:
  utils::Jiffies previous_process_cpu_time_ = {};
  utils::Jiffies previous_total_cpu_time_ = {};
  orbit_grpc_protos::ProcessInfo process_info_;
};

}  // namespace orbit_service

#endif  // ORBIT_SERVICE_PROCESS_H_
