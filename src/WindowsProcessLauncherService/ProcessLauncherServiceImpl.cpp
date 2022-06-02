// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsProcessLauncherService/ProcessLauncherServiceImpl.h"

#include <stdint.h>

#include <tuple>

#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Result.h"
#include "WindowsUtils/ProcessList.h"

namespace orbit_windows_process_launcher_service {

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::ProcessToLaunch;
using orbit_grpc_protos::ResumeProcessSuspendedAtEntryPointRequest;
using orbit_grpc_protos::ResumeProcessSuspendedAtEntryPointResponse;
using orbit_grpc_protos::SuspendProcessSpinningAtEntryPointRequest;
using orbit_grpc_protos::SuspendProcessSpinningAtEntryPointResponse;

using orbit_windows_utils::Process;

namespace {

[[nodiscard]] ProcessInfo ProcessInfoFromProcess(const Process* process) {
  ProcessInfo process_info;
  process_info.set_pid(process->pid);
  process_info.set_name(process->name);
  process_info.set_full_path(process->full_path);
  process_info.set_build_id(process->build_id);
  process_info.set_is_64_bit(process->is_64_bit);
  process_info.set_cpu_usage(process->cpu_usage_percentage);
  return process_info;
}

}  // namespace

grpc::Status WindowsProcessLauncherServiceImpl::LaunchProcess(
    grpc::ServerContext* context, const orbit_grpc_protos::LaunchProcessRequest* request,
    orbit_grpc_protos::LaunchProcessResponse* response) {
  const ProcessToLaunch& process_to_launch = request->process_to_launch();

  auto result = process_launcher_.LaunchProcess(
      process_to_launch.executable_path(), process_to_launch.working_directory(),
      process_to_launch.arguments(), process_to_launch.spin_at_entry_point());

  if (result.has_error()) {
    return Status(StatusCode::INVALID_ARGUMENT, result.error().message());
  }

  auto process_list_result = process_list_->Refresh();
  if (process_list_result.has_error()) {
    return Status(StatusCode::UNKNOWN, process_list_result.error().message());
  }

  uint32_t process_id = result.value();
  std::optional<const Process*> process = process_list_->GetProcessByPid(process_id);
  if (!process.has_value()) {
    // The process might have already exited.
    return Status(StatusCode::NOT_FOUND, "Launched process not found in process list");
  }

  *response->mutable_process_info() = std::move(ProcessInfoFromProcess(*process));
  return Status::OK;
}

grpc::Status WindowsProcessLauncherServiceImpl::SuspendProcessSpinningAtEntryPoint(
    grpc::ServerContext* context, const SuspendProcessSpinningAtEntryPointRequest* request,
    SuspendProcessSpinningAtEntryPointResponse* response) {
  auto result = process_launcher_.SuspendProcessSpinningAtEntryPoint(request->pid());
  if (result.has_error()) {
    return Status(StatusCode::NOT_FOUND, result.error().message());
  }

  return Status::OK;
}

grpc::Status WindowsProcessLauncherServiceImpl::ResumeProcessSuspendedAtEntryPoint(
    grpc::ServerContext* context, const ResumeProcessSuspendedAtEntryPointRequest* request,
    ResumeProcessSuspendedAtEntryPointResponse* response) {
  auto result = process_launcher_.ResumeProcessSuspendedAtEntryPoint(request->pid());
  if (result.has_error()) {
    return Status(StatusCode::NOT_FOUND, result.error().message());
  }

  return Status::OK;
}

}  // namespace orbit_windows_process_launcher_service
