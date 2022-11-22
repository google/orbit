// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PROCESS_SERVICE_PROCESS_H_
#define PROCESS_SERVICE_PROCESS_H_

#include <stdint.h>
#include <sys/types.h>

#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Result.h"
#include "ProcessService/CpuTime.h"

namespace orbit_process_service_internal {

class Process {
 public:
  void UpdateCpuUsage(Jiffies process_cpu_time, TotalCpuTime total_cpu_time);

  // Creates a `Process` by reading details from the `/proc` filesystem.
  // This might fail due to a non existing pid or due to permission problems.
  static ErrorMessageOr<Process> FromPid(uint32_t pid);

  // NOLINTNEXTLINE
  [[nodiscard]] const orbit_grpc_protos::ProcessInfo& process_info() const { return process_info_; }

 private:
  Jiffies previous_process_cpu_time_ = {};
  Jiffies previous_total_cpu_time_ = {};
  orbit_grpc_protos::ProcessInfo process_info_;
};

}  // namespace orbit_process_service_internal

#endif  // PROCESS_SERVICE_PROCESS_H_
