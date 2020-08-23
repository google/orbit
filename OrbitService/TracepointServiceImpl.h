// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TRACEPOINTSERVICEIMPL_H
#define ORBIT_TRACEPOINTSERVICEIMPL_H

#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_builder.h>

#include <memory>
#include <string>

#include "services.grpc.pb.h"

using grpc_impl::Server;
using grpc_impl::ServerBuilder;
using orbit_grpc_protos::TracepointService;

namespace orbit_service {

class TracepointServiceImpl final : public TracepointService::Service {
 public:
  [[nodiscard]] grpc::Status GetTracepointList(
      grpc::ServerContext* context, const orbit_grpc_protos::GetTracepointListRequest* request,
      orbit_grpc_protos::GetTracepointListResponse* response) override;
};

}  // namespace orbit_service
#endif  // ORBIT_TRACEPOINTSERVICEIMPL_H
