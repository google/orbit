// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CRASH_SERVICE_CRASH_SERVICE_IMPL_H_
#define CRASH_SERVICE_CRASH_SERVICE_IMPL_H_

#include <grpcpp/grpcpp.h>

#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"

namespace orbit_crash_service {

class CrashServiceImpl final : public orbit_grpc_protos::CrashService::Service {
 public:
  grpc::Status CrashOrbitService(grpc::ServerContext* context,
                                 const orbit_grpc_protos::CrashOrbitServiceRequest* request,
                                 orbit_grpc_protos::CrashOrbitServiceResponse* response) override;
};

}  // namespace orbit_crash_service

#endif  // CRASH_SERVICE_IMPL_CRASH_SERVICE_IMPL_H_
