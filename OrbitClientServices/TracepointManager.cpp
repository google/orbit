// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientServices/TracepointManager.h"

#include <OrbitBase/Logging.h>
#include <grpcpp/channel.h>

#include <thread>

#include "services.grpc.pb.h"

using orbit_grpc_protos::GetTracepointListRequest;
using orbit_grpc_protos::GetTracepointListResponse;
using orbit_grpc_protos::TracepointInfo;
using orbit_grpc_protos::TracepointService;

TracepointManager::TracepointManager(const std::shared_ptr<grpc::Channel>& channel)
    : tracepoint_service_(TracepointService::NewStub(channel)) {}

void TracepointManager::WorkerFunction() {
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

std::unique_ptr<grpc::ClientContext> TracepointManager::CreateContext() const {
  auto context = std::make_unique<grpc::ClientContext>();

  return context;
}

std::vector<TracepointInfo> TracepointManager::GetTracepointList() const {
  return tracepoint_list_;
}

std::unique_ptr<TracepointManager> TracepointManager::Create(
    const std::shared_ptr<grpc::Channel>& channel) {
  std::unique_ptr<TracepointManager> impl = std::make_unique<TracepointManager>(channel);
  impl->WorkerFunction();
  return impl;
}
