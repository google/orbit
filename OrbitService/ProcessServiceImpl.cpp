// TODO(http://b/148520406): Add copyright here
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessServiceImpl.h"

#include <memory>
#include <string>

using grpc::Status;
using grpc::ServerContext;

Status ProcessServiceImpl::GetProcessList(ServerContext*,
                                        const GetProcessListRequest*,
                                        GetProcessListReply* reply) {
  process_list_.Refresh();
  for (const std::shared_ptr<Process> process : process_list_.GetProcesses()) {
    ProcessInfo* process_info = reply->add_processes();
    process_info->set_pid(process->GetID());
    process_info->set_name(process->GetName());
    process_info->set_cpu_usage(process->GetCpuUsage());
  }

  return Status::OK;
}

