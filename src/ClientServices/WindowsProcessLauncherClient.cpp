// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientServices/WindowsProcessLauncherClient.h"

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/meta/type_traits.h>
#include <absl/synchronization/mutex.h>
#include <grpcpp/grpcpp.h>

#include <memory>

#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_services {
namespace {

using orbit_grpc_protos::LaunchProcessRequest;
using orbit_grpc_protos::LaunchProcessResponse;
using orbit_grpc_protos::ResumeProcessSuspendedAtEntryPointRequest;
using orbit_grpc_protos::ResumeProcessSuspendedAtEntryPointResponse;
using orbit_grpc_protos::SuspendProcessSpinningAtEntryPointRequest;
using orbit_grpc_protos::SuspendProcessSpinningAtEntryPointResponse;

class WindowsProcessLauncherClientImpl : public WindowsProcessLauncherClient {
 public:
  explicit WindowsProcessLauncherClientImpl(const std::shared_ptr<grpc::Channel>& channel)
      : windows_process_launcher_service_(
            orbit_grpc_protos::WindowsProcessLauncherService::NewStub(channel)) {}
  virtual ~WindowsProcessLauncherClientImpl() = default;

  ErrorMessageOr<orbit_grpc_protos::ProcessInfo> LaunchProcess(
      const orbit_grpc_protos::ProcessToLaunch& process_to_launch) override;
  ErrorMessageOr<void> SuspendProcessSpinningAtEntryPoint(uint32_t pid) override;
  ErrorMessageOr<void> ResumeProcessSuspendedAtEntryPoint(uint32_t pid) override;

  [[nodiscard]] bool IsProcessSpinningAtEntryPoint(uint32_t pid) override;
  [[nodiscard]] bool IsProcessSuspendedAtEntryPoint(uint32_t pid) override;

 private:
  struct LaunchedProcess {
    enum class State {
      kInvalid,
      kExecutingOrExited,
      kSpinningAtEntryPoint,
      kSuspendedAtEntryPoint
    };
    State state_ = State::kInvalid;
    orbit_grpc_protos::ProcessInfo process_info_;
  };

  void UpdateLaunchedProcessState(uint32_t pid, LaunchedProcess::State new_state);

  std::unique_ptr<orbit_grpc_protos::WindowsProcessLauncherService::Stub>
      windows_process_launcher_service_;

  absl::Mutex launched_processes_by_pid_mutex_;
  absl::flat_hash_map<uint32_t, LaunchedProcess> launched_processes_by_pid_
      ABSL_GUARDED_BY(launched_processes_by_pid_mutex_);
};

}  // namespace

ErrorMessageOr<orbit_grpc_protos::ProcessInfo> WindowsProcessLauncherClientImpl::LaunchProcess(
    const orbit_grpc_protos::ProcessToLaunch& process_to_launch) {
  ORBIT_SCOPE_FUNCTION;
  LaunchProcessRequest request;
  LaunchProcessResponse response;

  *request.mutable_process_to_launch() = process_to_launch;

  std::unique_ptr<grpc::ClientContext> context = std::make_unique<grpc::ClientContext>();
  grpc::Status status =
      windows_process_launcher_service_->LaunchProcess(context.get(), request, &response);

  if (!status.ok()) {
    ORBIT_ERROR("\"LaunchProcess\" gRPC call failed: code=%d, message=%s", status.error_code(),
                status.error_message());
    return ErrorMessage(status.error_message());
  }

  // Keep track of launched processes.
  const orbit_grpc_protos::ProcessInfo& process_info = response.process_info();
  LaunchedProcess launched_process;
  launched_process.process_info_ = process_info;
  launched_process.state_ = process_to_launch.spin_at_entry_point()
                                ? LaunchedProcess::State::kSpinningAtEntryPoint
                                : LaunchedProcess::State::kExecutingOrExited;
  {
    absl::MutexLock lock(&launched_processes_by_pid_mutex_);
    launched_processes_by_pid_.emplace(process_info.pid(), launched_process);
  }

  return process_info;
}

ErrorMessageOr<void> WindowsProcessLauncherClientImpl::SuspendProcessSpinningAtEntryPoint(
    uint32_t pid) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_CHECK(IsProcessSpinningAtEntryPoint(pid));
  SuspendProcessSpinningAtEntryPointRequest request;
  SuspendProcessSpinningAtEntryPointResponse response;
  request.set_pid(pid);

  std::unique_ptr<grpc::ClientContext> context = std::make_unique<grpc::ClientContext>();
  grpc::Status status = windows_process_launcher_service_->SuspendProcessSpinningAtEntryPoint(
      context.get(), request, &response);

  if (!status.ok()) {
    ORBIT_ERROR("\"SuspendProcessSpinningAtEntryPoint\" gRPC call failed: code=%d, message=%s",
                status.error_code(), status.error_message());
    return ErrorMessage(status.error_message());
  }

  UpdateLaunchedProcessState(pid, LaunchedProcess::State::kSuspendedAtEntryPoint);
  return outcome::success();
}

ErrorMessageOr<void> WindowsProcessLauncherClientImpl::ResumeProcessSuspendedAtEntryPoint(
    uint32_t pid) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_CHECK(IsProcessSuspendedAtEntryPoint(pid));
  ResumeProcessSuspendedAtEntryPointRequest request;
  ResumeProcessSuspendedAtEntryPointResponse response;
  request.set_pid(pid);

  std::unique_ptr<grpc::ClientContext> context = std::make_unique<grpc::ClientContext>();
  grpc::Status status = windows_process_launcher_service_->ResumeProcessSuspendedAtEntryPoint(
      context.get(), request, &response);

  if (!status.ok()) {
    ORBIT_ERROR("\"ResumeProcessSuspendedAtEntryPoint\" gRPC call failed: code=%d, message=%s",
                status.error_code(), status.error_message());
    return ErrorMessage(status.error_message());
  }

  UpdateLaunchedProcessState(pid, LaunchedProcess::State::kExecutingOrExited);
  return outcome::success();
}

void WindowsProcessLauncherClientImpl::UpdateLaunchedProcessState(
    uint32_t pid, LaunchedProcess::State new_state) {
  absl::MutexLock lock(&launched_processes_by_pid_mutex_);
  LaunchedProcess& launched_process = launched_processes_by_pid_.at(pid);
  launched_process.state_ = new_state;
}

bool WindowsProcessLauncherClientImpl::IsProcessSpinningAtEntryPoint(uint32_t pid) {
  absl::MutexLock lock(&launched_processes_by_pid_mutex_);
  auto launched_process_it = launched_processes_by_pid_.find(pid);
  if (launched_process_it == launched_processes_by_pid_.end()) return false;
  return launched_process_it->second.state_ == LaunchedProcess::State::kSpinningAtEntryPoint;
}

bool WindowsProcessLauncherClientImpl::IsProcessSuspendedAtEntryPoint(uint32_t pid) {
  absl::MutexLock lock(&launched_processes_by_pid_mutex_);
  auto launched_process_it = launched_processes_by_pid_.find(pid);
  if (launched_process_it == launched_processes_by_pid_.end()) return false;
  return launched_process_it->second.state_ == LaunchedProcess::State::kSuspendedAtEntryPoint;
}

std::unique_ptr<WindowsProcessLauncherClient> WindowsProcessLauncherClient::Create(
    const std::shared_ptr<grpc::Channel>& channel) {
  return std::make_unique<WindowsProcessLauncherClientImpl>(channel);
}

}  // namespace orbit_client_services
