// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessServiceImpl.h"

#include <memory>

using grpc::ServerContext;
using grpc::Status;

Status ProcessServiceImpl::GetProcessList(ServerContext*,
                                          const GetProcessListRequest*,
                                          GetProcessListResponse* response) {
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
