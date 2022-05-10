// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_SERVICES_PROCESS_CLIENT_H_
#define CLIENT_SERVICES_PROCESS_CLIENT_H_

#include <absl/types/span.h>
#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_client_services {

// This class handles the client calls related to process service.
class ProcessClient {
 public:
  virtual ~ProcessClient() = default;

  virtual ErrorMessageOr<std::vector<orbit_grpc_protos::ProcessInfo>> GetProcessList() = 0;
  virtual ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> LoadModuleList(
      uint32_t pid) = 0;
  virtual ErrorMessageOr<std::string> FindDebugInfoFile(
      const std::string& module_path,
      absl::Span<const std::string> additional_search_directories) = 0;
  virtual ErrorMessageOr<std::string> LoadProcessMemory(uint32_t pid, uint64_t address,
                                                        uint64_t size) = 0;

  virtual ErrorMessageOr<orbit_grpc_protos::ProcessInfo> LaunchProcess(
      const orbit_grpc_protos::ProcessToLaunch& process_to_launch) = 0;
  virtual ErrorMessageOr<void> SuspendProcessSpinningAtEntryPoint(uint32_t pid) = 0;
  virtual ErrorMessageOr<void> ResumeProcessSuspendedAtEntryPoint(uint32_t pid) = 0;

  [[nodiscard]] static std::unique_ptr<ProcessClient> Create(
      const std::shared_ptr<grpc::Channel>& channel);
};

}  // namespace orbit_client_services

#endif  // CLIENT_SERVICES_PROCESS_CLIENT_H_
