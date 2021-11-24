// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_SERVICES_CRASH_MANAGER_H_
#define CLIENT_SERVICES_CRASH_MANAGER_H_

#include <grpcpp/grpcpp.h>

#include <memory>

#include "GrpcProtos/services.pb.h"

namespace orbit_client_services {

// This class is responsible for crash OrbitService.
//
// Usage example:
//
// auto manager = CrashManager::Create(...);
//
// To crash OrbitService:
// manager.CrashOrbitService(...);
//
class CrashManager {
 public:
  CrashManager() = default;
  virtual ~CrashManager() = default;

  virtual void CrashOrbitService(
      orbit_grpc_protos::CrashOrbitServiceRequest_CrashType crash_type) = 0;

  static std::unique_ptr<CrashManager> Create(const std::shared_ptr<grpc::Channel>& channel);
};

}  // namespace orbit_client_services

#endif  // CLIENT_SERVICES_CRASH_MANAGER_H_
