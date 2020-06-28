// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "Message.h"
#include "TcpEntity.h"

class ConnectionManager {
 public:
  ConnectionManager();
  ~ConnectionManager();
  static ConnectionManager& Get();
  void ConnectToRemote(std::string a_RemoteAddress);
  void Stop();

 private:
  void ConnectionThreadWorker();

  void StopThread();
  void SetupClientCallbacks();

  std::thread thread_;
  std::string remote_address_;
  std::atomic<bool> exit_requested_;
};
