// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessServiceImpl.h"

#include <memory>

#include "SymbolHelper.h"
#include "symbol.pb.h"

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

Status ProcessServiceImpl::GetProcessList(ServerContext*,
                                          const GetProcessListRequest*,
                                          GetProcessListResponse* response) {
  absl::MutexLock lock(&mutex_);

  process_list_.Refresh();
  process_list_.UpdateCpuTimes();
  for (const std::shared_ptr<Process>& process : process_list_.GetProcesses()) {
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
  int32_t pid = request->process_id();
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
    module_info->set_build_id(module->m_DebugSignature);
  }

  return Status::OK;
}

Status ProcessServiceImpl::GetProcessMemory(
    ServerContext*, const GetProcessMemoryRequest* request,
    GetProcessMemoryResponse* response) {
  uint64_t size = std::min(request->size(), kMaxGetProcessMemoryResponseSize);
  response->mutable_memory()->resize(size);
  uint64_t num_bytes_read = 0;
  if (ReadProcessMemory(
          request->pid(), request->address(),
          reinterpret_cast<uint8_t*>(response->mutable_memory()->data()), size,
          &num_bytes_read)) {
    response->mutable_memory()->resize(num_bytes_read);
    return Status::OK;
  } else {
    response->mutable_memory()->resize(0);
    ERROR("GetProcessMemory: reading %lu bytes from address %#lx of process %u",
          size, request->address(), request->pid());
    return Status(
        StatusCode::PERMISSION_DENIED,
        absl::StrFormat(
            "Could not read %lu bytes from address %#lx of process %u", size,
            request->address(), request->pid()));
  }
}

Status ProcessServiceImpl::GetSymbols(ServerContext*,
                                      const GetSymbolsRequest* request,
                                      GetSymbolsResponse* response) {
  // TODO(antonrohr) remove this need for Module. SymbolHelper needs to be
  // changed for that.
  std::shared_ptr<Module> module = std::make_shared<Module>();
  module->m_FullName = request->module_path();

  const SymbolHelper symbol_helper;
  const auto load_result =
      symbol_helper.LoadSymbolsCollector(request->module_path());

  if (!load_result) return Status(StatusCode::NOT_FOUND, load_result.error());

  *response->mutable_module_symbols() = std::move(load_result.value());

  LOG("Loaded %lu symbols for module %s, (size: %d bytes)",
      response->module_symbols().symbol_infos().size(), request->module_path(),
      response->ByteSize());

  return Status::OK;
}
