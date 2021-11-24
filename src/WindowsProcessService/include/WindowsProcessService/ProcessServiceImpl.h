// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_PROCESS_SERVICE_PROCESS_SERVICE_IMPL_H_
#define WINDOWS_PROCESS_SERVICE_PROCESS_SERVICE_IMPL_H_

#include <absl/synchronization/mutex.h>
#include <grpcpp/grpcpp.h>

#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "WindowsUtils/ProcessList.h"

namespace orbit_windows_process_service {

class ProcessServiceImpl final : public orbit_grpc_protos::ProcessService::Service {
 public:
  [[nodiscard]] grpc::Status GetProcessList(
      grpc::ServerContext* context, const orbit_grpc_protos::GetProcessListRequest* request,
      orbit_grpc_protos::GetProcessListResponse* response) override;

  [[nodiscard]] grpc::Status GetModuleList(
      grpc::ServerContext* context, const orbit_grpc_protos::GetModuleListRequest* request,
      orbit_grpc_protos::GetModuleListResponse* response) override;

  [[nodiscard]] grpc::Status GetProcessMemory(
      grpc::ServerContext* context, const orbit_grpc_protos::GetProcessMemoryRequest* request,
      orbit_grpc_protos::GetProcessMemoryResponse* response) override;

  [[nodiscard]] grpc::Status GetDebugInfoFile(
      grpc::ServerContext* context, const orbit_grpc_protos::GetDebugInfoFileRequest* request,
      orbit_grpc_protos::GetDebugInfoFileResponse* response) override;

 private:
  absl::Mutex mutex_;
  std::unique_ptr<orbit_windows_utils::ProcessList> process_list_;

  static constexpr size_t kMaxGetProcessMemoryResponseSize = 8 * 1024 * 1024;
};

}  // namespace orbit_windows_process_service

#endif  // WINDOWS_PROCESS_SERVICE_PROCESS_SERVICE_IMPL_H_
