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

void TracepointServiceClient::WorkerFunction() {
  GetTracepointListRequest request;
  GetTracepointListResponse response;
  std::unique_ptr<grpc::ClientContext> context = CreateContext();

  grpc::Status status = tracepoint_service_->GetTracepointList(context.get(), request, &response);
  if (!status.ok()) {
    ERROR("gRPC call to GetTracepointList failed: %s (error_code=%d)", status.error_message(),
          status.error_code());
    return;
  }

  const auto& tracepoints = response.tracepoints();
  tracepoint_list_.assign(tracepoints.begin(), tracepoints.end());
}

std::unique_ptr<grpc::ClientContext> TracepointServiceClient::CreateContext() const {
  auto context = std::make_unique<grpc::ClientContext>();

  return context;
}

std::vector<TracepointInfo> TracepointServiceClient::GetTracepointList() const {
  return tracepoint_list_;
}

std::unique_ptr<TracepointServiceClient> TracepointServiceClient::Create(
    const std::shared_ptr<grpc::Channel>& channel) {
  std::unique_ptr<TracepointServiceClient> impl =
      std::make_unique<TracepointServiceClient>(channel);
  impl->WorkerFunction();
  return impl;
}
