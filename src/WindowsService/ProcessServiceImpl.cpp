// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessServiceImpl.h"

#include <OrbitLib/OrbitLib.h>
#include <absl/base/casts.h>
#include <absl/strings/str_format.h>
#include <stdint.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <vector>

#include "ObjectUtils/CoffFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "WindowsUtils/ListModules.h"
#include "WindowsUtils/ListProcesses.h"
#include "module.pb.h"
#include "process.pb.h"

namespace windows_service {

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
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;

using orbit_windows_utils::Module;
using orbit_windows_utils::Process;

Status ProcessServiceImpl::GetProcessList(ServerContext*, const GetProcessListRequest*,
                                          GetProcessListResponse* response) {
  const std::vector<Process> processes = orbit_windows_utils::ListProcesses();
  if (processes.empty()) {
    return Status(StatusCode::NOT_FOUND, "Error listing processes");
  }

  for (const Process& process : processes) {
    ProcessInfo* process_info = response->add_processes();
    process_info->set_pid(process.pid);
    process_info->set_name(process.name);
    process_info->set_full_path(process.full_path);
    process_info->set_build_id(process.build_id);
    process_info->set_is_64_bit(process.is_64_bit);
  }

  return Status::OK;
}

Status ProcessServiceImpl::GetModuleList(ServerContext* /*context*/,
                                         const GetModuleListRequest* request,
                                         GetModuleListResponse* response) {
  std::vector<Module> modules = orbit_windows_utils::ListModules(request->process_id());
  if (modules.empty()) {
    return Status(StatusCode::NOT_FOUND, "Error listing modules");
  }

  for (const Module& module : modules) {
    ModuleInfo* module_info = response->add_modules();
    module_info->set_name(module.name);
    module_info->set_file_path(module.full_path);
    module_info->set_address_start(module.address_start);
    module_info->set_address_end(module.address_end);
    module_info->set_build_id(module.build_id);
  }
  return Status::OK;
}

Status ProcessServiceImpl::GetProcessMemory(ServerContext*, const GetProcessMemoryRequest* request,
                                            GetProcessMemoryResponse* response) {
  HANDLE process_handle = OpenProcess(PROCESS_VM_READ, FALSE, request->pid());
  if (process_handle == nullptr) {
    return Status(StatusCode::PERMISSION_DENIED,
                  absl::StrFormat("Could not get handle for process %u", request->pid()));
  }

  uint64_t size = std::min(request->size(), kMaxGetProcessMemoryResponseSize);
  response->mutable_memory()->resize(size);
  void* address = absl::bit_cast<void*>(request->address());
  uint64_t num_bytes_read = 0;
  auto result = ReadProcessMemory(process_handle, address, response->mutable_memory()->data(), size,
                                  &num_bytes_read);
  response->mutable_memory()->resize(num_bytes_read);

  if (result != 0) {
    return Status::OK;
  }

  return Status(StatusCode::PERMISSION_DENIED,
                absl::StrFormat("Could not read %lu bytes from address %#lx of process %u", size,
                                request->address(), request->pid()));
}

Status ProcessServiceImpl::GetDebugInfoFile(ServerContext*, const GetDebugInfoFileRequest* request,
                                            GetDebugInfoFileResponse* response) {
  // TODO-PG
  return Status(StatusCode::NOT_FOUND, "");
  // response->set_debug_info_file_path(symbols_path.value());
  // return Status::OK;
}

}  // namespace windows_service
