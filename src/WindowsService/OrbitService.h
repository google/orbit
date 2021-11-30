// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_SERVICE_ORBIT_SERVICE_H
#define WINDOWS_SERVICE_ORBIT_SERVICE_H

#include <stdint.h>

#include <atomic>

#include "GrpcProtos/capture.pb.h"

namespace orbit_windows_service {

class OrbitService {
 public:
  explicit OrbitService(uint16_t grpc_port, bool dev_mode)
      : grpc_port_{grpc_port}, dev_mode_{dev_mode} {}

  void Run(std::atomic<bool>* exit_requested);

 private:
  uint16_t grpc_port_;
  bool dev_mode_;
};

}  // namespace orbit_windows_service

#endif  // WINDOWS_SERVICE_ORBIT_SERVICE_H
