// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_PROCESS_H_
#define ORBIT_SERVICE_PROCESS_H_

#include "OrbitBase/Result.h"
#include "process.pb.h"

namespace orbit_service {

class Process : public orbit_grpc_protos::ProcessInfo {
 public:
  using orbit_grpc_protos::ProcessInfo::ProcessInfo;

  // Creates a `Process` by reading details from the `/proc` filesystem.
  // This might fail due to a non existing pid or due to permission problems.
  static ErrorMessageOr<Process> FromPidAndCpuUsage(pid_t pid, double cpu_usage);
};

}  // namespace orbit_service

#endif  // ORBIT_SERVICE_PROCESS_H_
