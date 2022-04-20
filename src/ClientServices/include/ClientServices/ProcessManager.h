// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_SERVICES_PROCESS_MANAGER_H_
#define CLIENT_SERVICES_PROCESS_MANAGER_H_

#include <absl/time/time.h>
#include <absl/types/span.h>
#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "GrpcProtos/services.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "OrbitBase/Result.h"
#include "absl/synchronization/mutex.h"

namespace orbit_client_services {

// ProcessManager is used to access or launch processes on the target machine through OrbitService.
class ProcessManager {
 public:
  ProcessManager() = default;
  virtual ~ProcessManager() = default;

  // Set callback to periodically receive the list of processes running on the target.
  virtual void SetProcessListUpdateListener(
      const std::function<void(std::vector<orbit_grpc_protos::ProcessInfo>)>& listener) = 0;

  virtual ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> LoadModuleList(
      uint32_t pid) = 0;

  virtual ErrorMessageOr<std::string> LoadProcessMemory(uint32_t pid, uint64_t address,
                                                        uint64_t size) = 0;

  virtual ErrorMessageOr<std::string> LoadNullTerminatedString(uint32_t pid, uint64_t address) = 0;

  virtual ErrorMessageOr<std::string> FindDebugInfoFile(
      const std::string& module_path,
      absl::Span<const std::string> additional_search_directories) = 0;

  // Process launching is only implemented on Windows for now.
  // TODO(https://b/232009293): Profile from entry point on Stadia/Linux.
#ifdef _WIN32
  virtual ErrorMessageOr<orbit_grpc_protos::ProcessInfo> LaunchProcess(
      const orbit_grpc_protos::ProcessToLaunch& process_to_launch) = 0;
  [[nodiscard]] virtual bool IsProcessSpinningAtEntryPoint(uint32_t pid) = 0;
  [[nodiscard]] virtual bool IsProcessSuspendedAtEntryPoint(uint32_t pid) = 0;
  virtual void SuspendProcessSpinningAtEntryPoint(uint32_t pid) = 0;
  virtual void ResumeProcessSuspendedAtEntryPoint(uint32_t pid) = 0;
#endif

  // Create ProcessManager with specified refresh period.
  static std::unique_ptr<ProcessManager> Create(const std::shared_ptr<grpc::Channel>& channel,
                                                absl::Duration refresh_timeout);
};

}  // namespace orbit_client_services

#endif  // CLIENT_SERVICES_PROCESS_MANAGER_H_
