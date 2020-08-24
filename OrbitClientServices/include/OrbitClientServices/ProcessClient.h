// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_SERVICES_PROCESS_CLIENT_H_
#define ORBIT_CLIENT_SERVICES_PROCESS_CLIENT_H_

#include <string>

#include "OrbitBase/Result.h"
#include "grpcpp/grpcpp.h"
#include "services.grpc.pb.h"

class ProcessClient {
 public:
  explicit ProcessClient(const std::shared_ptr<grpc::Channel>& channel)
      : process_service_(orbit_grpc_protos::ProcessService::NewStub(channel)) {}

  [[nodiscard]] ErrorMessageOr<std::vector<orbit_grpc_protos::ProcessInfo>> GetProcessList();

  [[nodiscard]] ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> LoadModuleList(
      int32_t pid);

  [[nodiscard]] ErrorMessageOr<std::vector<orbit_grpc_protos::TracepointInfo>> LoadTracepointList();

  [[nodiscard]] ErrorMessageOr<std::string> FindDebugInfoFile(const std::string& module_path);

  [[nodiscard]] ErrorMessageOr<std::string> LoadProcessMemory(int32_t pid, uint64_t address,
                                                              uint64_t size);

 private:
  std::unique_ptr<orbit_grpc_protos::ProcessService::Stub> process_service_;
};

#endif  // ORBIT_CLIENT_SERVICES_PROCESS_CLIENT_H_