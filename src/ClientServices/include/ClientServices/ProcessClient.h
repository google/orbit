// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_SERVICES_PROCESS_CLIENT_H_
#define CLIENT_SERVICES_PROCESS_CLIENT_H_

#include <absl/types/span.h>
#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"

namespace orbit_client_services {

// This class handles the client calls related to process service.
class ProcessClient {
 public:
  explicit ProcessClient(const std::shared_ptr<grpc::Channel>& channel)
      : process_service_(orbit_grpc_protos::ProcessService::NewStub(channel)) {}

  [[nodiscard]] ErrorMessageOr<std::vector<orbit_grpc_protos::ProcessInfo>> GetProcessList();

  [[nodiscard]] ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> LoadModuleList(
      uint32_t pid);

  [[nodiscard]] ErrorMessageOr<orbit_base::NotFoundOr<std::filesystem::path>> FindDebugInfoFile(
      std::string_view module_path, absl::Span<const std::string> additional_search_directories);

  [[nodiscard]] ErrorMessageOr<std::string> LoadProcessMemory(uint32_t pid, uint64_t address,
                                                              uint64_t size);

 private:
  std::unique_ptr<orbit_grpc_protos::ProcessService::Stub> process_service_;
};

}  // namespace orbit_client_services

#endif  // CLIENT_SERVICES_PROCESS_CLIENT_H_
