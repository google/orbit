// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientServices/ProcessClient.h"

#include <absl/types/span.h>
#include <grpcpp/grpcpp.h>

#include <chrono>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_services {
namespace {

using orbit_grpc_protos::GetDebugInfoFileRequest;
using orbit_grpc_protos::GetDebugInfoFileResponse;
using orbit_grpc_protos::GetModuleListRequest;
using orbit_grpc_protos::GetModuleListResponse;
using orbit_grpc_protos::GetProcessListRequest;
using orbit_grpc_protos::GetProcessListResponse;
using orbit_grpc_protos::GetProcessMemoryRequest;
using orbit_grpc_protos::GetProcessMemoryResponse;
using orbit_grpc_protos::LaunchProcessRequest;
using orbit_grpc_protos::LaunchProcessResponse;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::ResumeProcessSuspendedAtEntryPointRequest;
using orbit_grpc_protos::ResumeProcessSuspendedAtEntryPointResponse;
using orbit_grpc_protos::SuspendProcessSpinningAtEntryPointRequest;
using orbit_grpc_protos::SuspendProcessSpinningAtEntryPointResponse;

constexpr uint64_t kGrpcDefaultTimeoutMilliseconds = 3000;

class ProcessClientImpl : public ProcessClient {
 public:
  explicit ProcessClientImpl(const std::shared_ptr<grpc::Channel>& channel)
      : process_service_(orbit_grpc_protos::ProcessService::NewStub(channel)) {}
  virtual ~ProcessClientImpl() = default;

  ErrorMessageOr<std::vector<orbit_grpc_protos::ProcessInfo>> GetProcessList() override;

  ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> LoadModuleList(uint32_t pid) override;

  ErrorMessageOr<std::string> FindDebugInfoFile(
      const std::string& module_path,
      absl::Span<const std::string> additional_search_directories) override;

  ErrorMessageOr<std::string> LoadProcessMemory(uint32_t pid, uint64_t address,
                                                uint64_t size) override;

  ErrorMessageOr<orbit_grpc_protos::ProcessInfo> LaunchProcess(
      const orbit_grpc_protos::ProcessToLaunch& process_to_launch) override;
  ErrorMessageOr<void> SuspendProcessSpinningAtEntryPoint(uint32_t pid) override;
  ErrorMessageOr<void> ResumeProcessSuspendedAtEntryPoint(uint32_t pid) override;

 private:
  std::unique_ptr<orbit_grpc_protos::ProcessService::Stub> process_service_;
};

std::unique_ptr<grpc::ClientContext> CreateContext(
    uint64_t timeout_milliseconds = kGrpcDefaultTimeoutMilliseconds) {
  auto context = std::make_unique<grpc::ClientContext>();
  std::chrono::system_clock::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_milliseconds);
  context->set_deadline(deadline);

  return context;
}

}  // namespace

ErrorMessageOr<std::vector<orbit_grpc_protos::ProcessInfo>> ProcessClientImpl::GetProcessList() {
  ORBIT_SCOPE_FUNCTION;
  GetProcessListRequest request;
  GetProcessListResponse response;
  std::unique_ptr<grpc::ClientContext> context = CreateContext();

  grpc::Status status = process_service_->GetProcessList(context.get(), request, &response);
  if (!status.ok()) {
    ORBIT_ERROR("gRPC call to GetProcessList failed: %s (error_code=%d)", status.error_message(),
                status.error_code());
    return ErrorMessage(status.error_message());
  }

  const auto& processes = response.processes();

  return std::vector<ProcessInfo>(processes.begin(), processes.end());
}

ErrorMessageOr<std::vector<ModuleInfo>> ProcessClientImpl::LoadModuleList(uint32_t pid) {
  ORBIT_SCOPE_FUNCTION;
  GetModuleListRequest request;
  GetModuleListResponse response;
  request.set_process_id(pid);

  std::unique_ptr<grpc::ClientContext> context = CreateContext();
  grpc::Status status = process_service_->GetModuleList(context.get(), request, &response);

  if (!status.ok()) {
    ORBIT_ERROR("\"LoadModuleList\" grpc call failed: code=%d, message=%s", status.error_code(),
                status.error_message());
    return ErrorMessage(status.error_message());
  }

  const auto& modules = response.modules();

  return std::vector<ModuleInfo>(modules.begin(), modules.end());
}

ErrorMessageOr<std::string> ProcessClientImpl::FindDebugInfoFile(
    const std::string& module_path, absl::Span<const std::string> additional_search_directories) {
  ORBIT_SCOPE_FUNCTION;
  GetDebugInfoFileRequest request;
  GetDebugInfoFileResponse response;

  request.set_module_path(module_path);
  *request.mutable_additional_search_directories() = {additional_search_directories.begin(),
                                                      additional_search_directories.end()};

  std::unique_ptr<grpc::ClientContext> context = CreateContext();

  grpc::Status status = process_service_->GetDebugInfoFile(context.get(), request, &response);
  if (!status.ok()) {
    ORBIT_ERROR("gRPC call to GetDebugInfoFile failed: %s", status.error_message());
    return ErrorMessage(status.error_message());
  }

  return response.debug_info_file_path();
}

ErrorMessageOr<std::string> ProcessClientImpl::LoadProcessMemory(uint32_t pid, uint64_t address,
                                                                 uint64_t size) {
  ORBIT_SCOPE_FUNCTION;
  GetProcessMemoryRequest request;
  request.set_pid(pid);
  request.set_address(address);
  request.set_size(size);

  GetProcessMemoryResponse response;

  std::unique_ptr<grpc::ClientContext> context = CreateContext();

  grpc::Status status = process_service_->GetProcessMemory(context.get(), request, &response);
  if (!status.ok()) {
    ORBIT_ERROR("\"GetProcessMemory\" grpc call failed: code=%d, message=%s", status.error_code(),
                status.error_message());
    return ErrorMessage(status.error_message());
  }

  return std::move(*response.mutable_memory());
}

[[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ProcessInfo> ProcessClientImpl::LaunchProcess(
    const orbit_grpc_protos::ProcessToLaunch& process_to_launch) {
  LaunchProcessRequest request;
  LaunchProcessResponse response;

  *request.mutable_process_to_launch() = process_to_launch;

  std::unique_ptr<grpc::ClientContext> context = CreateContext();
  grpc::Status status = process_service_->LaunchProcess(context.get(), request, &response);

  if (!status.ok()) {
    ORBIT_ERROR("\"LaunchProcess\" grpc call failed: code=%d, message=%s", status.error_code(),
                status.error_message());
    return ErrorMessage(status.error_message());
  }

  return response.process_info();
}

ErrorMessageOr<void> ProcessClientImpl::SuspendProcessSpinningAtEntryPoint(uint32_t pid) {
  SuspendProcessSpinningAtEntryPointRequest request;
  SuspendProcessSpinningAtEntryPointResponse response;
  request.set_pid(pid);

  std::unique_ptr<grpc::ClientContext> context = CreateContext();
  grpc::Status status =
      process_service_->SuspendProcessSpinningAtEntryPoint(context.get(), request, &response);

  if (!status.ok()) {
    ORBIT_ERROR("\"SuspendProcessSpinningAtEntryPoint\" grpc call failed: code=%d, message=%s",
                status.error_code(), status.error_message());
    return ErrorMessage(status.error_message());
  }

  return outcome::success();
}

ErrorMessageOr<void> ProcessClientImpl::ResumeProcessSuspendedAtEntryPoint(uint32_t pid) {
  ResumeProcessSuspendedAtEntryPointRequest request;
  ResumeProcessSuspendedAtEntryPointResponse response;
  request.set_pid(pid);

  std::unique_ptr<grpc::ClientContext> context = CreateContext();
  grpc::Status status =
      process_service_->ResumeProcessSuspendedAtEntryPoint(context.get(), request, &response);

  if (!status.ok()) {
    ORBIT_ERROR("\"ResumeProcessSuspendedAtEntryPoint\" grpc call failed: code=%d, message=%s",
                status.error_code(), status.error_message());
    return ErrorMessage(status.error_message());
  }

  return outcome::success();
}

std::unique_ptr<ProcessClient> ProcessClient::Create(
    const std::shared_ptr<grpc::Channel>& channel) {
  return std::make_unique<ProcessClientImpl>(channel);
}

}  // namespace orbit_client_services
