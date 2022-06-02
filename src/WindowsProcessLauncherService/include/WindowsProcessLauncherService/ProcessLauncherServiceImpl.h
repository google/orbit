// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_PROCESS_LAUNCHER_SERVICE_PROCESS_LAUNCHER_SERVICE_IMPL_H_
#define WINDOWS_PROCESS_LAUNCHER_SERVICE_PROCESS_LAUNCHER_SERVICE_IMPL_H_

#include <grpcpp/grpcpp.h>

#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "WindowsUtils/ProcessLauncher.h"
#include "WindowsUtils/ProcessList.h"

namespace orbit_windows_process_launcher_service {

// WindowsProcessLauncherServiceImpl wraps orbit_windows_utils::ProcessLauncher in a grpc service.
// LaunchProcess can start the process in paused mode such that it is spinning at the entry point.
// To remove the busy loop but remain paused at entry, we call "SuspendProcessSpinningAtEntryPoint".
// "ResumeProcessSuspendedAtEntryPointRequest" can then be called to resume normal execution.
class WindowsProcessLauncherServiceImpl final
    : public orbit_grpc_protos::WindowsProcessLauncherService::Service {
 public:
  [[nodiscard]] grpc::Status LaunchProcess(
      grpc::ServerContext* context, const orbit_grpc_protos::LaunchProcessRequest* request,
      orbit_grpc_protos::LaunchProcessResponse* response) override;

  [[nodiscard]] grpc::Status SuspendProcessSpinningAtEntryPoint(
      grpc::ServerContext* context,
      const orbit_grpc_protos::SuspendProcessSpinningAtEntryPointRequest* request,
      orbit_grpc_protos::SuspendProcessSpinningAtEntryPointResponse* response) override;

  [[nodiscard]] grpc::Status ResumeProcessSuspendedAtEntryPoint(
      grpc::ServerContext* context,
      const orbit_grpc_protos::ResumeProcessSuspendedAtEntryPointRequest* request,
      orbit_grpc_protos::ResumeProcessSuspendedAtEntryPointResponse* response) override;

 private:
  std::unique_ptr<orbit_windows_utils::ProcessList> process_list_;
  orbit_windows_utils::ProcessLauncher process_launcher_;
};

}  // namespace orbit_windows_process_launcher_service

#endif  // WINDOWS_PROCESS_SERVICE_PROCESS_SERVICE_IMPL_H_
