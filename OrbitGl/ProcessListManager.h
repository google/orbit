// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PROCESS_MANAGER_H_
#define ORBIT_GL_PROCESS_MANAGER_H_

#include <memory>
#include <string>
#include <thread>

#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "grpcpp/grpcpp.h"
#include "process.pb.h"

// This class is resposible for maintaining
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
// manager.SetCallback(callback);
//
// manager.Start();
//
// To orderly shutdown the manager use following:
//
// manager.Shutdown();
//
class ProcessListManager {
 public:
  ProcessListManager() = default;
  virtual ~ProcessListManager() = default;

  virtual void SetCallback(
      const std::function<void(std::vector<ProcessInfo>&&)>& listener) = 0;
  virtual void Start() = 0;
  // Note that this method waits for the worker thread to stop, which could
  // take up to refresh_timeout.
  virtual void Shutdown() = 0;

  // Create ProcessListManager with specified duration
  static std::unique_ptr<ProcessListManager> Create(
      std::shared_ptr<grpc::Channel> channel, absl::Duration refresh_timeout);
};

#endif  // ORBIT_GL_PROCESS_MANAGER_H_
