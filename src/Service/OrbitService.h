// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_ORBIT_SERVICE_H
#define ORBIT_SERVICE_ORBIT_SERVICE_H

#include <stdint.h>

#include <atomic>
#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "GrpcProtos/capture.pb.h"

namespace orbit_service {

class OrbitService {
 public:
  explicit OrbitService(uint16_t grpc_port, bool dev_mode)
      : grpc_port_{grpc_port}, dev_mode_{dev_mode} {}

  [[nodiscard]] int Run(std::atomic<bool>* exit_requested);

 private:
  [[nodiscard]] bool IsSshWatchdogActive() { return last_stdin_message_ != std::nullopt; }

  uint16_t grpc_port_;
  bool dev_mode_;

  std::optional<std::chrono::time_point<std::chrono::steady_clock>> last_stdin_message_ =
      std::nullopt;
  const std::string_view kStartWatchdogPassphrase = "start_watchdog";
  // TODO(antonrohr): The main thread can currently be blocked by slow functions
  //  like FunctionsDataView::DoSort and FunctionsDataView::DoFilter. The
  //  default timeout of 10 seconds is not enough with the blocking behaviour.
  //  As soon as the main thread does not block anymore, revert this from 25
  //  seconds back to 10 seconds.
  const int kWatchdogTimeoutInSeconds = 25;
};

}  // namespace orbit_service

#endif  // ORBIT_SERVICE_ORBIT_SERVICE_H
