// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointService/TracepointServiceImpl.h"

#include <vector>

#include "GrpcProtos/services.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "ReadTracepoints.h"

namespace orbit_tracepoint_service {

grpc::Status TracepointServiceImpl::GetTracepointList(grpc::ServerContext* /*context*/,
                                                      const GetTracepointListRequest* /*request*/,
                                                      GetTracepointListResponse* response) {
  ORBIT_LOG("Sending tracepoints");

  const auto tracepoint_infos = ReadTracepoints();
  if (tracepoint_infos.has_error()) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, tracepoint_infos.error().message());
  }

  *response->mutable_tracepoints() = {tracepoint_infos.value().begin(),
                                      tracepoint_infos.value().end()};

  return grpc::Status::OK;
}

}  // namespace orbit_tracepoint_service
