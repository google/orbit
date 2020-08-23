// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TRACEPOINTSERVICEIMPL_H
#define ORBIT_TRACEPOINTSERVICEIMPL_H

#include <memory>
#include <string>

#include "services.grpc.pb.h"

namespace orbit_service {

using orbit_grpc_protos::TracepointService;
using orbit_grpc_protos::GetTracepointListRequest;
using orbit_grpc_protos::GetTracepointListResponse;

class TracepointServiceImpl final : public TracepointService::Service {
 public:

  [[nodiscard]] grpc::Status GetTracepointList(
      grpc::ServerContext* context, const GetTracepointListRequest* request,
      GetTracepointListResponse* response) override;

  //void RunServer() {};
};

}
#endif  // ORBIT_TRACEPOINTSERVICEIMPL_H
