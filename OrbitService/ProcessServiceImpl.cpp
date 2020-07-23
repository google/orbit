// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessServiceImpl.h"

#include <memory>

#include "LinuxUtils.h"
#include "OrbitBase/Logging.h"
#include "SymbolHelper.h"
#include "Utils.h"
#include "symbol.pb.h"

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

Status ProcessServiceImpl::GetProcessList(ServerContext*,
                                          const GetProcessListRequest*,
                                          GetProcessListResponse* response) {
  {
    absl::MutexLock lock(&mutex_);

    const auto refresh_result = process_list_.Refresh();
    if (!refresh_result) {
      return Status(StatusCode::INTERNAL, refresh_result.error());
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

Status ProcessServiceImpl::GetModuleList(ServerContext*,
                                         const GetModuleListRequest* request,
                                         GetModuleListResponse* response) {
  int32_t pid = request->process_id();
  LOG("Sending modules for process %d", pid);

  const auto module_infos = LinuxUtils::ListModules(pid);
  if (!module_infos) {
    return Status(StatusCode::NOT_FOUND, module_infos.error().message());
  }

  for (const auto& module_info : module_infos.value()) {
    *(response->add_modules()) = module_info;
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
  const SymbolHelper symbol_helper;
  const auto load_result =
      symbol_helper.LoadSymbolsCollector(request->module_path());

  if (load_result.has_error()) {
    return Status(StatusCode::NOT_FOUND, load_result.error().message());
  }

  *response->mutable_module_symbols() = std::move(load_result.value());

  LOG("Loaded %lu symbols for module \"%s\" (size: %d bytes)",
      response->module_symbols().symbol_infos().size(), request->module_path(),
      response->ByteSize());

  return Status::OK;
}
Status ProcessServiceImpl::GetDebugInfoFile(
    ::grpc::ServerContext*, const ::GetDebugInfoFileRequest* request,
    ::GetDebugInfoFileResponse* response) {
  const SymbolHelper symbol_helper;
  ErrorMessageOr<std::string> result = symbol_helper.FindDebugSymbolsFile(
      request->module_path(), request->build_id());
  if (!result) {
    return Status(StatusCode::NOT_FOUND, result.error().message());
  }

  response->set_debug_info_file_path(result.value());

  return Status::OK;
}
