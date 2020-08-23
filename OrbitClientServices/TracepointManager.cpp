// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientServices/TracepointManager.h"

#include <OrbitBase/Logging.h>
#include <grpcpp/channel.h>

#include <thread>

#include "services.grpc.pb.h"

namespace {

using orbit_grpc_protos::GetTracepointListRequest;
using orbit_grpc_protos::GetTracepointListResponse;
using orbit_grpc_protos::TracepointInfo;
using orbit_grpc_protos::TracepointService;

constexpr uint64_t kGrpcDefaultTimeoutMilliseconds = 1000;

class TracepointManagerImpl final : public TracepointManager {
  ErrorMessageOr<std::vector<TracepointInfo>> LoadTracepointList() override;

 public:
  explicit TracepointManagerImpl(const std::shared_ptr<grpc::Channel>& channel,
                                 absl::Duration refresh_timeout);
 private:
  std::unique_ptr<TracepointService::Stub> tracepoint_service_;

  std::unique_ptr<grpc::ClientContext> CreateContext(uint64_t timeout_milliseconds) const;
  void WorkerFunction();

  absl::Duration refresh_timeout_;
  mutable absl::Mutex mutex_;
};

TracepointManagerImpl::TracepointManagerImpl(const std::shared_ptr<grpc::Channel>& channel,
                                             absl::Duration refresh_timeout)
    : tracepoint_service_(TracepointService::NewStub(channel)), refresh_timeout_(refresh_timeout) {}

ErrorMessageOr<std::vector<TracepointInfo>> TracepointManagerImpl::LoadTracepointList() {
  GetTracepointListRequest request;
  GetTracepointListResponse response;

  std::unique_ptr<grpc::ClientContext> context = CreateContext(kGrpcDefaultTimeoutMilliseconds);
  grpc::Status status = tracepoint_service_->GetTracepointList(context.get(), request, &response);

  if (!status.ok()) {
    ERROR("Grpc call failed: code=%d, message=%s", status.error_code(), status.error_message());
    return ErrorMessage(status.error_message());
  }

  const auto& tracepoints = response.tracepoints();

  return std::vector<TracepointInfo>(tracepoints.begin(), tracepoints.end());
}

static std::unique_ptr<TracepointManager> Create(const std::shared_ptr<grpc::Channel>& channel,
                                                 absl::Duration refresh_timeout){
  std::unique_ptr<TracepointManagerImpl> impl =
      std::make_unique<TracepointManagerImpl>(channel, refresh_timeout);
  impl->Start();
  return impl;
}

std::unique_ptr<grpc::ClientContext> TracepointManagerImpl::CreateContext(
    uint64_t timeout_milliseconds) const {
  auto context = std::make_unique<grpc::ClientContext>();

  std::chrono::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_milliseconds);
  context->set_deadline(deadline);

  return context;
}

}  // namespace
