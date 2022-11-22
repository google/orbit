// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessService/ProcessServiceImpl.h"

#include <absl/strings/str_format.h>
#include <stdint.h>
#include <sys/types.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "GrpcProtos/services.pb.h"
#include "ModuleUtils/ReadLinuxModules.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"
#include "ProcessServiceUtils.h"

namespace orbit_process_service {

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

using orbit_base::NotFoundOr;

using orbit_grpc_protos::GetDebugInfoFileRequest;
using orbit_grpc_protos::GetDebugInfoFileResponse;
using orbit_grpc_protos::GetModuleListRequest;
using orbit_grpc_protos::GetModuleListResponse;
using orbit_grpc_protos::GetProcessListRequest;
using orbit_grpc_protos::GetProcessListResponse;
using orbit_grpc_protos::GetProcessMemoryRequest;
using orbit_grpc_protos::GetProcessMemoryResponse;
using orbit_grpc_protos::ProcessInfo;

Status ProcessServiceImpl::GetProcessList(ServerContext* /*context*/,
                                          const GetProcessListRequest* /*request*/,
                                          GetProcessListResponse* response) {
  {
    absl::MutexLock lock(&mutex_);

    const auto refresh_result = process_list_.Refresh();
    if (refresh_result.has_error()) {
      return {StatusCode::INTERNAL, refresh_result.error().message()};
    }
  }

  const std::vector<ProcessInfo>& processes = process_list_.GetProcesses();
  if (processes.empty()) {
    return {StatusCode::NOT_FOUND, "Error while getting processes."};
  }

  for (const auto& process_info : process_list_.GetProcesses()) {
    *(response->add_processes()) = process_info;
  }

  return Status::OK;
}

Status ProcessServiceImpl::GetModuleList(ServerContext* /*context*/,
                                         const GetModuleListRequest* request,
                                         GetModuleListResponse* response) {
  pid_t pid = orbit_base::ToNativeProcessId(request->process_id());
  ORBIT_LOG("Sending modules for process %d", pid);

  const auto module_infos = orbit_module_utils::ReadModules(pid);
  if (module_infos.has_error()) {
    return {StatusCode::NOT_FOUND, module_infos.error().message()};
  }

  for (const auto& module_info : module_infos.value()) {
    *(response->add_modules()) = module_info;
  }

  return Status::OK;
}

Status ProcessServiceImpl::GetProcessMemory(ServerContext* /*context*/,
                                            const GetProcessMemoryRequest* request,
                                            GetProcessMemoryResponse* response) {
  uint64_t size = std::min(request->size(), kMaxGetProcessMemoryResponseSize);
  response->mutable_memory()->resize(size);
  uint64_t num_bytes_read = 0;
  if (ReadProcessMemory(request->pid(), request->address(), response->mutable_memory()->data(),
                        size, &num_bytes_read)) {
    response->mutable_memory()->resize(num_bytes_read);
    return Status::OK;
  }

  response->mutable_memory()->resize(0);
  ORBIT_ERROR("GetProcessMemory: reading %lu bytes from address %#lx of process %u", size,
              request->address(), request->pid());
  return {StatusCode::PERMISSION_DENIED,
          absl::StrFormat("Could not read %lu bytes from address %#lx of process %u", size,
                          request->address(), request->pid())};
}

Status ProcessServiceImpl::GetDebugInfoFile(ServerContext* /*context*/,
                                            const GetDebugInfoFileRequest* request,
                                            GetDebugInfoFileResponse* response) {
  ORBIT_CHECK(request != nullptr);

  const ErrorMessageOr<NotFoundOr<std::filesystem::path>>& find_result_or_error =
      FindSymbolsFilePath(*request);
  if (find_result_or_error.has_error()) {
    return {StatusCode::UNKNOWN, find_result_or_error.error().message()};
  }

  const NotFoundOr<std::filesystem::path>& find_result = find_result_or_error.value();

  if (orbit_base::IsNotFound(find_result)) {
    return Status{StatusCode::NOT_FOUND, orbit_base::GetNotFoundMessage(find_result)};
  }
  response->set_debug_info_file_path(orbit_base::GetFound(find_result));
  return Status::OK;
}

}  // namespace orbit_process_service
