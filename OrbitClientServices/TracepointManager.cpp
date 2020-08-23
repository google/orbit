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

 public:
  explicit TracepointManagerImpl(const std::shared_ptr<grpc::Channel>& channel,
                        absl::Duration refresh_timeout);

  ErrorMessageOr<std::vector<TracepointInfo>> LoadTracepointList() override;

  std::vector<TracepointInfo> GetTracepointList() const override;


  void Start();
 private:

  std::unique_ptr<grpc::ClientContext> CreateContext(uint64_t timeout_milliseconds) const;
  void WorkerFunction();
  std::unique_ptr<TracepointService::Stub> tracepoint_service_;

  absl::Duration refresh_timeout_;
  mutable absl::Mutex mutex_;

  absl::Mutex shutdown_mutex_;
  bool shutdown_initiated_;

  std::vector<TracepointInfo> tracepoint_list_;

  std::thread worker_thread_;
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

bool IsTrue(bool* var) { return *var; }

void TracepointManagerImpl::WorkerFunction() {

    if (shutdown_mutex_.LockWhenWithTimeout(absl::Condition(IsTrue, &shutdown_initiated_),
                                            refresh_timeout_)) {
      // Shutdown was initiated we need to exit
      shutdown_mutex_.Unlock();
      return;
    }
    shutdown_mutex_.Unlock();
    // Timeout expired - refresh the list

    GetTracepointListRequest request;
    GetTracepointListResponse response;
    std::unique_ptr<grpc::ClientContext> context = CreateContext(kGrpcDefaultTimeoutMilliseconds);

    grpc::Status status = tracepoint_service_->GetTracepointList(context.get(), request, &response);
    if (!status.ok()) {
      ERROR("gRPC call to GetTracepointList failed: %s (error_code=%d)", status.error_message(),
            status.error_code());
      return;
    }

    absl::MutexLock callback_lock(&mutex_);
    const auto& processes = response.tracepoints();
    tracepoint_list_.assign(processes.begin(), processes.end());

}

void TracepointManagerImpl::Start() {
  CHECK(!worker_thread_.joinable());
  worker_thread_ = std::thread([this] { WorkerFunction(); });

}

std::unique_ptr<grpc::ClientContext> TracepointManagerImpl::CreateContext(
    uint64_t timeout_milliseconds) const {
  auto context = std::make_unique<grpc::ClientContext>();

  std::chrono::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_milliseconds);
  context->set_deadline(deadline);

  return context;
}

std::vector<TracepointInfo> TracepointManagerImpl::GetTracepointList() const {
  absl::MutexLock lock(&mutex_);
  return tracepoint_list_;
}

}  // namespace

std::unique_ptr<TracepointManager> TracepointManager::Create(const std::shared_ptr<grpc::Channel>& channel,
                                                                    absl::Duration refresh_timeout) {
  std::unique_ptr<TracepointManagerImpl> impl =
      std::make_unique<TracepointManagerImpl>(channel, refresh_timeout);
  impl->Start();
  return impl;
}
