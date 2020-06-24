// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERIVICE_CRASH_SERVICE_IMPL_H_
#define ORBIT_SERIVICE_CRASH_SERVICE_IMPL_H_

#include "services.grpc.pb.h"

class CrashServiceImpl final : public CrashService::Service {
 public:
  grpc::Status CrashOrbitService(grpc::ServerContext* context,
                                 const GetCrashRequest* request,
                                 GetCrashResponse* response) override;
};

#endif  // ORBIT_SERIVICE_CRASH_SERVICE_IMPL_H_
