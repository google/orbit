// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessServiceImpl.h"

#include <memory>
#include <string>

#include "OrbitModule.h"

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

Status ProcessServiceImpl::GetProcessList(ServerContext*,
                                          const GetProcessListRequest*,
                                          GetProcessListResponse* response) {
  absl::MutexLock lock(&mutex_);

  process_list_.Refresh();
  process_list_.UpdateCpuTimes();
  for (const std::shared_ptr<Process> process : process_list_.GetProcesses()) {
    ProcessInfo* process_info = response->add_processes();
    process_info->set_pid(process->GetID());
    process_info->set_name(process->GetName());
    process_info->set_cpu_usage(process->GetCpuUsage());
    process_info->set_full_path(process->GetFullPath());
    process_info->set_command_line(process->GetCmdLine());
    process_info->set_is_64_bit(process->GetIs64Bit());
  }

  return Status::OK;
}

Status ProcessServiceImpl::GetModuleList(ServerContext*,
                                         const GetModuleListRequest* request,
                                         GetModuleListResponse* response) {
  uint32_t pid = request->process_id();
  LOG("Sending modules for process %d", pid);

  absl::MutexLock lock(&mutex_);

  std::shared_ptr<Process> process = process_list_.GetProcess(pid);
  if (process == nullptr) {
    return Status(
        StatusCode::NOT_FOUND,
        absl::StrFormat("The process with pid=%d was not found", pid));
  }

  process->ListModules();

  const std::map<uint64_t, std::shared_ptr<Module>>& modules =
      process->GetModules();

  for (auto& it : modules) {
    std::shared_ptr<Module> module = it.second;
    ModuleInfo* module_info = response->add_modules();

    module_info->set_name(module->m_Name);
    module_info->set_file_path(module->m_FullName);
    module_info->set_file_size(module->m_PdbSize);
    module_info->set_address_start(module->m_AddressStart);
    module_info->set_address_end(module->m_AddressEnd);
  }

  return Status::OK;
}
