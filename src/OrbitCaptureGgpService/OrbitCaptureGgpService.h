// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CAPTURE_GGP_SERVICE_ORBIT_CAPTURE_GGP_SERVICE_H_
#define ORBIT_CAPTURE_GGP_SERVICE_ORBIT_CAPTURE_GGP_SERVICE_H_

#include <grpcpp/grpcpp.h>
#include <stdint.h>

class OrbitCaptureGgpService final {
 public:
  explicit OrbitCaptureGgpService(uint16_t grpc_port) : grpc_port_{grpc_port} {}

  void RunServer() const;

 private:
  uint16_t grpc_port_;
};

#endif  // ORBIT_CAPTURE_GGP_SERVICE_ORBIT_CAPTURE_GGP_SERVICE_H_