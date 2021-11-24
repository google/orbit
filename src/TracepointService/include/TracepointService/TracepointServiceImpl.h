// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TRACEPOINT_SERVICE_TRACEPOINT_SERVICE_IMPL_
#define TRACEPOINT_SERVICE_TRACEPOINT_SERVICE_IMPL_

#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>

#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"

namespace orbit_tracepoint_service {

using orbit_grpc_protos::GetTracepointListRequest;
using orbit_grpc_protos::GetTracepointListResponse;
using orbit_grpc_protos::TracepointService;

class TracepointServiceImpl final : public TracepointService::Service {
 public:
  [[nodiscard]] grpc::Status GetTracepointList(grpc::ServerContext* context,
                                               const GetTracepointListRequest* request,
                                               GetTracepointListResponse* response) override;
};

}  // namespace orbit_tracepoint_service

#endif  // TRACEPOINT_SERVICE_TRACEPOINT_SERVICE_IMPL_
