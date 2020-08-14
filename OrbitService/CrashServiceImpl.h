// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_CRASH_SERVICE_IMPL_H_
#define ORBIT_SERVICE_CRASH_SERVICE_IMPL_H_

#include "services.grpc.pb.h"

namespace orbit_service {

class CrashServiceImpl final : public orbit_grpc_protos::CrashService::Service {
 public:
  grpc::Status CrashOrbitService(grpc::ServerContext* context,
                                 const orbit_grpc_protos::CrashOrbitServiceRequest* request,
                                 orbit_grpc_protos::CrashOrbitServiceResponse* response) override;
};

}  // namespace orbit_service

#endif  // ORBIT_SERVICE_CRASH_SERVICE_IMPL_H_
