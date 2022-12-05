// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_SERVICES_PROCESS_MANAGER_H_
#define CLIENT_SERVICES_PROCESS_MANAGER_H_

#include <absl/time/time.h>
#include <absl/types/span.h>
#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"
#include "absl/synchronization/mutex.h"

namespace orbit_client_services {

// This class is responsible for maintaining
// process list. It periodically updates it
// and calls callback to notify listeners when
// the list is updated.
//
// Usage example:
//
// auto manager = ProcessListManager::Create(...);
// auto callback = [&](const std::vector<ProcessInfo>& process_list) {
//   // Update process list in UI
// }
// manager.SetProcessListUpdateListener(callback);
//
// To orderly shutdown the manager use following:
//
// manager.Shutdown();
//
class ProcessManager {
 public:
  ProcessManager() = default;
  virtual ~ProcessManager() = default;

  virtual void SetProcessListUpdateListener(
      const std::function<void(std::vector<orbit_grpc_protos::ProcessInfo>)>& listener) = 0;

  virtual ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> LoadModuleList(
      uint32_t pid) = 0;

  virtual ErrorMessageOr<std::string> LoadProcessMemory(uint32_t pid, uint64_t address,
                                                        uint64_t size) = 0;

  virtual ErrorMessageOr<orbit_base::NotFoundOr<std::filesystem::path>> FindDebugInfoFile(
      std::string_view module_path,
      absl::Span<const std::string> additional_search_directories) = 0;

  // Note that this method waits for the worker thread to stop, which could
  // take up to refresh_timeout.
  virtual void ShutdownAndWait() = 0;

  // Create ProcessManager with specified duration
  static std::unique_ptr<ProcessManager> Create(const std::shared_ptr<grpc::Channel>& channel,
                                                absl::Duration refresh_timeout);
};

}  // namespace orbit_client_services

#endif  // CLIENT_SERVICES_PROCESS_MANAGER_H_
