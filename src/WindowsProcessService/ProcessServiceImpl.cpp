// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsProcessService/ProcessServiceImpl.h"

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <stdint.h>

#include <filesystem>
#include <string>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "ObjectUtils/ObjectFile.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "WindowsTracing/ListModulesETW.h"
#include "WindowsUtils/FindDebugSymbols.h"
#include "WindowsUtils/ListModules.h"
#include "WindowsUtils/ProcessList.h"
#include "WindowsUtils/ReadProcessMemory.h"

namespace orbit_windows_process_service {

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
  std::vector<const Process*> processes;

  {
    absl::MutexLock lock(&mutex_);
    if (process_list_ == nullptr) {
      process_list_ = orbit_windows_utils::ProcessList::Create();
    }
    auto result = process_list_->Refresh();
    if (result.has_error()) {
      return Status(StatusCode::NOT_FOUND,
                    absl::StrFormat("Error listing processes: %s", result.error().message()));
    }
    processes = process_list_->GetProcesses();
    ORBIT_CHECK(!processes.empty());
  }

  for (const Process* process : processes) {
    ProcessInfo* process_info = response->add_processes();
    process_info->set_pid(process->pid);
    process_info->set_name(process->name);
    process_info->set_full_path(process->full_path);
    process_info->set_build_id(process->build_id);
    process_info->set_is_64_bit(process->is_64_bit);
    process_info->set_cpu_usage(process->cpu_usage_percentage);
  }

  return Status::OK;
}

Status ProcessServiceImpl::GetModuleList(ServerContext* /*context*/,
                                         const GetModuleListRequest* request,
                                         GetModuleListResponse* response) {
  std::vector<Module> modules = orbit_windows_utils::ListModules(request->process_id());
  if (modules.empty()) {
    // Fallback on etw module enumeration which involves more work.
    modules = orbit_windows_tracing::ListModulesEtw(request->process_id());
  }

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
    module_info->set_load_bias(module.load_bias);
    module_info->set_object_file_type(ModuleInfo::kCoffFile);
    for (const ModuleInfo::ObjectSegment& section : module.sections) {
      *module_info->add_object_segments() = section;
    }
  }

  return Status::OK;
}

Status ProcessServiceImpl::GetProcessMemory(ServerContext*, const GetProcessMemoryRequest* request,
                                            GetProcessMemoryResponse* response) {
  const uint64_t size = std::min(request->size(), kMaxGetProcessMemoryResponseSize);
  ErrorMessageOr<std::string> result =
      orbit_windows_utils::ReadProcessMemory(request->pid(), request->address(), size);

  if (result.has_error()) {
    return Status(StatusCode::PERMISSION_DENIED, result.error().message());
  }

  *response->mutable_memory() = std::move(result.value());
  return Status::OK;
}

Status ProcessServiceImpl::GetDebugInfoFile(ServerContext*, const GetDebugInfoFileRequest* request,
                                            GetDebugInfoFileResponse* response) {
  std::filesystem::path module_path(request->module_path());

  const ErrorMessageOr<std::filesystem::path> symbols_path =
      orbit_windows_utils::FindDebugSymbols(module_path, {});

  if (symbols_path.has_error()) {
    return Status(StatusCode::NOT_FOUND, symbols_path.error().message());
  }

  response->set_debug_info_file_path(symbols_path.value().string());
  return Status::OK;
}

}  // namespace orbit_windows_process_service
