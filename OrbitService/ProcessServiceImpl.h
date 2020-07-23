// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_PROCESS_SERVICE_IMPL_H_
#define ORBIT_SERVICE_PROCESS_SERVICE_IMPL_H_

#include <absl/synchronization/mutex.h>

#include <memory>
#include <string>

#include "ProcessList.h"
#include "services.grpc.pb.h"

class ProcessServiceImpl final : public ProcessService::Service {
 public:
  grpc::Status GetProcessList(grpc::ServerContext* context,
                              const GetProcessListRequest* request,
                              GetProcessListResponse* response) override;
  grpc::Status GetSymbols(grpc::ServerContext* context,
                          const GetSymbolsRequest* request,
                          GetSymbolsResponse* response) override;

  grpc::Status GetModuleList(grpc::ServerContext* context,
                             const GetModuleListRequest* request,
                             GetModuleListResponse* response) override;

  grpc::Status GetProcessMemory(grpc::ServerContext* context,
                                const GetProcessMemoryRequest* request,
                                GetProcessMemoryResponse* response) override;

  grpc::Status GetDebugInfoFile(grpc::ServerContext* context,
                                const GetDebugInfoFileRequest* request,
                                GetDebugInfoFileResponse* response) override;

 private:
  absl::Mutex mutex_;
  ProcessList process_list_;

  static constexpr size_t kMaxGetProcessMemoryResponseSize = 8 * 1024 * 1024;
};

#endif  // ORBIT_SERVICE_PROCESS_SERVICE_IMPL_H_
