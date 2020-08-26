// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointServiceImpl.h"

#include "OrbitBase/Logging.h"
#include "Utils.h"
#include "grpcpp/grpcpp.h"
#include "services.pb.h"

namespace orbit_service {

grpc::Status TracepointServiceImpl::GetTracepointList(grpc::ServerContext*,
                                                      const GetTracepointListRequest*,
                                                      GetTracepointListResponse* response) {
  LOG("Sending tracepoints");

  const auto tracepoint_infos = utils::ReadTracepoints();
  if (!tracepoint_infos) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, tracepoint_infos.error().message());
  }

  *response->mutable_tracepoints() = {tracepoint_infos.value().begin(),
                                      tracepoint_infos.value().end()};

  return grpc::Status::OK;
}

}  // namespace orbit_service
