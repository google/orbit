// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientServices/ProcessClient.h"

#include <memory>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Tracing.h"
#include "grpcpp/grpcpp.h"
#include "outcome.hpp"
#include "services.grpc.pb.h"
#include "symbol.pb.h"

namespace {

using orbit_grpc_protos::GetDebugInfoFileRequest;
using orbit_grpc_protos::GetDebugInfoFileResponse;
using orbit_grpc_protos::GetModuleListRequest;
using orbit_grpc_protos::GetModuleListResponse;
using orbit_grpc_protos::GetProcessListRequest;
using orbit_grpc_protos::GetProcessListResponse;
using orbit_grpc_protos::GetProcessMemoryRequest;
using orbit_grpc_protos::GetProcessMemoryResponse;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::ProcessService;

constexpr uint64_t kGrpcDefaultTimeoutMilliseconds = 3000;

std::unique_ptr<grpc::ClientContext> CreateContext(
    uint64_t timeout_milliseconds = kGrpcDefaultTimeoutMilliseconds) {
  auto context = std::make_unique<grpc::ClientContext>();
  std::chrono::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_milliseconds);
  context->set_deadline(deadline);

  return context;
}

}  // namespace

ErrorMessageOr<std::vector<orbit_grpc_protos::ProcessInfo>> ProcessClient::GetProcessList() {
  ORBIT_SCOPE_FUNCTION;
  GetProcessListRequest request;
  GetProcessListResponse response;
  std::unique_ptr<grpc::ClientContext> context = CreateContext();

  grpc::Status status = process_service_->GetProcessList(context.get(), request, &response);
  if (!status.ok()) {
    ERROR("gRPC call to GetProcessList failed: %s (error_code=%d)", status.error_message(),
          status.error_code());
    return ErrorMessage(status.error_message());
  }

  const auto& processes = response.processes();

  return std::vector<ProcessInfo>(processes.begin(), processes.end());
}

ErrorMessageOr<std::vector<ModuleInfo>> ProcessClient::LoadModuleList(int32_t pid) {
  ORBIT_SCOPE_FUNCTION;
  GetModuleListRequest request;
  GetModuleListResponse response;
  request.set_process_id(pid);

  std::unique_ptr<grpc::ClientContext> context = CreateContext();
  grpc::Status status = process_service_->GetModuleList(context.get(), request, &response);

  if (!status.ok()) {
    ERROR("Grpc call failed: code=%d, message=%s", status.error_code(), status.error_message());
    return ErrorMessage(status.error_message());
  }

  const auto& modules = response.modules();

  return std::vector<ModuleInfo>(modules.begin(), modules.end());
}

ErrorMessageOr<std::string> ProcessClient::FindDebugInfoFile(const std::string& module_path) {
  ORBIT_SCOPE_FUNCTION;
  GetDebugInfoFileRequest request;
  GetDebugInfoFileResponse response;

  request.set_module_path(module_path);

  std::unique_ptr<grpc::ClientContext> context = CreateContext();

  grpc::Status status = process_service_->GetDebugInfoFile(context.get(), request, &response);
  if (!status.ok()) {
    ERROR("gRPC call to GetDebugInfoFile failed: %s", status.error_message());
    return ErrorMessage(status.error_message());
  }

  return response.debug_info_file_path();
}

ErrorMessageOr<std::string> ProcessClient::LoadProcessMemory(int32_t pid, uint64_t address,
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
    ERROR("gRPC call to GetProcessMemory failed: %s", status.error_message());
    return ErrorMessage(status.error_message());
  }

  return std::move(*response.mutable_memory());
}
