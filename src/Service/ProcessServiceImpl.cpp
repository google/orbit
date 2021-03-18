// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessServiceImpl.h"

#include <absl/strings/str_format.h>
#include <stdint.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <vector>

#include "ElfUtils/LinuxMap.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "ServiceUtils.h"
#include "module.pb.h"
#include "process.pb.h"

namespace orbit_service {

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

using orbit_grpc_protos::GetDebugInfoFileRequest;
using orbit_grpc_protos::GetDebugInfoFileResponse;
using orbit_grpc_protos::GetModuleListRequest;
using orbit_grpc_protos::GetModuleListResponse;
using orbit_grpc_protos::GetProcessListRequest;
using orbit_grpc_protos::GetProcessListResponse;
using orbit_grpc_protos::GetProcessMemoryRequest;
using orbit_grpc_protos::GetProcessMemoryResponse;
using orbit_grpc_protos::ProcessInfo;

Status ProcessServiceImpl::GetProcessList(ServerContext*, const GetProcessListRequest*,
                                          GetProcessListResponse* response) {
  {
    absl::MutexLock lock(&mutex_);

    const auto refresh_result = process_list_.Refresh();
    if (refresh_result.has_error()) {
      return Status(StatusCode::INTERNAL, refresh_result.error().message());
    }
  }

  const std::vector<ProcessInfo>& processes = process_list_.GetProcesses();
  if (processes.empty()) {
    return Status(StatusCode::NOT_FOUND, "Error while getting processes.");
  }

  for (const auto& process_info : process_list_.GetProcesses()) {
    *(response->add_processes()) = process_info;
  }

  return Status::OK;
}

Status ProcessServiceImpl::GetModuleList(ServerContext* /*context*/,
                                         const GetModuleListRequest* request,
                                         GetModuleListResponse* response) {
  int32_t pid = request->process_id();
  LOG("Sending modules for process %d", pid);

  const auto module_infos = orbit_elf_utils::ReadModules(pid);
  if (module_infos.has_error()) {
    return Status(StatusCode::NOT_FOUND, module_infos.error().message());
  }

  for (const auto& module_info : module_infos.value()) {
    *(response->add_modules()) = module_info;
  }

  return Status::OK;
}

Status ProcessServiceImpl::GetProcessMemory(ServerContext*, const GetProcessMemoryRequest* request,
                                            GetProcessMemoryResponse* response) {
  uint64_t size = std::min(request->size(), kMaxGetProcessMemoryResponseSize);
  response->mutable_memory()->resize(size);
  const auto result = utils::ReadProcessMemory(request->pid(), request->address(),
                                               response->mutable_memory()->data(), size);
  if (result.has_value()) {
    response->mutable_memory()->resize(result.value());
    return Status::OK;
  }

  response->mutable_memory()->resize(0);
  ERROR("GetProcessMemory: reading %lu bytes from address %#lx of process %u", size,
        request->address(), request->pid());
  return Status(StatusCode::PERMISSION_DENIED,
                absl::StrFormat("Could not read %lu bytes from address %#lx of process %u", size,
                                request->address(), request->pid()));
}

Status ProcessServiceImpl::GetDebugInfoFile(ServerContext*, const GetDebugInfoFileRequest* request,
                                            GetDebugInfoFileResponse* response) {
  const auto symbols_path = utils::FindSymbolsFilePath(request->module_path());
  if (symbols_path.has_error()) {
    return Status(StatusCode::NOT_FOUND, symbols_path.error().message());
  }

  response->set_debug_info_file_path(symbols_path.value());
  return Status::OK;
}

}  // namespace orbit_service