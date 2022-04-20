// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_SERVICES_LAUNCHED_PROCESS_
#define CLIENT_SERVICES_LAUNCHED_PROCESS_

#include "ClientServices/ProcessClient.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_client_services {

// LaunchedProcess holds client side information about a process that has been launched from
// OrbitService, possibly spinning at entry point, and manages its state transitions.
class LaunchedProcess {
 public:
  // A LaunchedProcess can only be created by launching a process.
  static ErrorMessageOr<std::unique_ptr<LaunchedProcess>> LaunchProcess(
      const orbit_grpc_protos::ProcessToLaunch& process_to_launch, ProcessClient* client);

  // Prevent other creation methods.
  LaunchedProcess() = delete;
  LaunchedProcess(const LaunchedProcess&) = delete;
  LaunchedProcess(const LaunchedProcess&&) = delete;
  LaunchedProcess& operator=(const LaunchedProcess&) = delete;
  LaunchedProcess& operator=(const LaunchedProcess&&) = delete;

  // Suspend a process spinning at entry point and restore its original instructions.
  [[nodiscard]] ErrorMessageOr<void> SuspendProcessSpinningAtEntryPoint(ProcessClient* client);
  // Resume a process suspended at entry point.
  [[nodiscard]] ErrorMessageOr<void> ResumeProcessSuspendedAtEntryPoint(ProcessClient* client);
  // Get process info.
  [[nodiscard]] const orbit_grpc_protos::ProcessInfo& GetProcessInfo() const;
  // Get current state of the launched process.
  [[nodiscard]] bool IsProcessSpinningAtEntryPoint() const;
  [[nodiscard]] bool IsProcessSuspendedAtEntryPoint() const;
  [[nodiscard]] bool IsProcessExecutingOrExited() const;

 private:
  enum class State { kInvalid, kExecutingOrExited, kSpinningAtEntryPoint, kSuspendedAtEntryPoint };
  LaunchedProcess(State initial_state, orbit_grpc_protos::ProcessInfo process_info);
  State state_ = State::kInvalid;
  orbit_grpc_protos::ProcessInfo process_info_;
};

}  // namespace orbit_client_services

#endif  // CLIENT_SERVICES_LAUNCHED_PROCESS_
