// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientServices/TracepointServiceClient.h"

#include "OrbitBase/Logging.h"

using orbit_grpc_protos::GetTracepointListRequest;
using orbit_grpc_protos::GetTracepointListResponse;
using orbit_grpc_protos::TracepointInfo;
using orbit_grpc_protos::TracepointService;

TracepointServiceClient::TracepointServiceClient(const std::shared_ptr<grpc::Channel>& channel)
    : tracepoint_service_(TracepointService::NewStub(channel)) {}

std::unique_ptr<grpc::ClientContext> TracepointServiceClient::CreateContext() const {
  auto context = std::make_unique<grpc::ClientContext>();

  return context;
}

std::vector<TracepointInfo> TracepointServiceClient::GetTracepointList() const {
  GetTracepointListRequest request;
  GetTracepointListResponse response;
  std::unique_ptr<grpc::ClientContext> context = CreateContext();

  std::vector<TracepointInfo> tracepoint_list;
  grpc::Status status = tracepoint_service_->GetTracepointList(context.get(), request, &response);
  if (!status.ok()) {
    ERROR("gRPC call to GetTracepointList failed: %s (error_code=%d)", status.error_message(),
          status.error_code());
  }

  else {
    const auto& tracepoints = response.tracepoints();
    tracepoint_list.assign(tracepoints.begin(), tracepoints.end());
  }
  return tracepoint_list;
}

std::unique_ptr<TracepointServiceClient> TracepointServiceClient::Create(
    const std::shared_ptr<grpc::Channel>& channel) {
  std::unique_ptr<TracepointServiceClient> client =
      std::make_unique<TracepointServiceClient>(channel);
  return client;
}
