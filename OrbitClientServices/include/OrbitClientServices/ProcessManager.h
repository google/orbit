// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_SERVICES_PROCESS_MANAGER_H_
#define ORBIT_CLIENT_SERVICES_PROCESS_MANAGER_H_

#include <tracepoint.pb.h>

#include <memory>
#include <string>
#include <thread>

#include "OrbitBase/Result.h"
#include "absl/synchronization/mutex.h"
#include "grpcpp/grpcpp.h"
#include "module.pb.h"
#include "process.pb.h"
#include "symbol.pb.h"

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
      const std::function<void(ProcessManager*)>& listener) = 0;

  virtual ErrorMessageOr<std::vector<ModuleInfo>> LoadModuleList(
      int32_t pid) = 0;

  virtual ErrorMessageOr<std::vector<TracepointInfo>> LoadTracepointList() = 0;

  // Get a copy of process list.
  virtual std::vector<ProcessInfo> GetProcessList() const = 0;

  virtual ErrorMessageOr<std::string> LoadProcessMemory(int32_t pid,
                                                        uint64_t address,
                                                        uint64_t size) = 0;

  virtual ErrorMessageOr<std::string> LoadNullTerminatedString(
      int32_t pid, uint64_t address) = 0;

  virtual ErrorMessageOr<std::string> FindDebugInfoFile(
      const std::string& module_path, const std::string& build_id) = 0;

  // Note that this method waits for the worker thread to stop, which could
  // take up to refresh_timeout.
  virtual void Shutdown() = 0;

  // Create ProcessManager with specified duration
  static std::unique_ptr<ProcessManager> Create(
      const std::shared_ptr<grpc::Channel>& channel,
      absl::Duration refresh_timeout);
};

#endif  // ORBIT_CLIENT_SERVICES_PROCESS_MANAGER_H_
