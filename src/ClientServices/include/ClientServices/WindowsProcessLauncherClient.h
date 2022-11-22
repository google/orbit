// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_SERVICES_WINDOWS_PROCESS_LAUNCHER_CLIENT_H_
#define CLIENT_SERVICES_WINDOWS_PROCESS_LAUNCHER_CLIENT_H_

#include <absl/types/span.h>
#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_client_services {

// This class handles the client calls related to launching a Windows process. This class is
// thread-safe.
class WindowsProcessLauncherClient {
 public:
  virtual ~WindowsProcessLauncherClient() = default;

  // Launch a Windows process.
  virtual ErrorMessageOr<orbit_grpc_protos::ProcessInfo> LaunchProcess(
      const orbit_grpc_protos::ProcessToLaunch& process_to_launch) = 0;
  // Suspend a process spinning at entry point and restore its original instructions.
  virtual ErrorMessageOr<void> SuspendProcessSpinningAtEntryPoint(uint32_t pid) = 0;
  // Resume a process suspended at entry point.
  virtual ErrorMessageOr<void> ResumeProcessSuspendedAtEntryPoint(uint32_t pid) = 0;

  [[nodiscard]] virtual bool IsProcessSpinningAtEntryPoint(uint32_t pid) = 0;
  [[nodiscard]] virtual bool IsProcessSuspendedAtEntryPoint(uint32_t pid) = 0;

  [[nodiscard]] static std::unique_ptr<WindowsProcessLauncherClient> Create(
      const std::shared_ptr<grpc::Channel>& channel);
};

}  // namespace orbit_client_services

#endif  // CLIENT_SERVICES_WINDOWS_PROCESS_CLIENT_H_
