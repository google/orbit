// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CRASH_MANAGER_H_
#define ORBIT_GL_CRASH_MANAGER_H_

#include <memory>

#include "grpcpp/grpcpp.h"
#include "services.pb.h"

// This class is responsible for crash OrbitService.
//
// Usage example:
//
// auto manager = CrashManager::Create(...);
//
// To crash OrbitService:
// manager.CrashOrbitService(...);
//
//
class CrashManager {
 public:
  CrashManager() = default;
  virtual ~CrashManager() = default;

  virtual void CrashOrbitService(
      CrashOrbitServiceRequest_CrashType crash_type) = 0;

  static std::unique_ptr<CrashManager> Create(
      std::shared_ptr<grpc::Channel> channel);
};

#endif  // ORBIT_GL_CRASH_MANAGER_H_
