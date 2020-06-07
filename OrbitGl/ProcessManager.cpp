// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessManager.h"

#include <chrono>
#include <memory>
#include <string>

#include "OrbitBase/Logging.h"
#include "grpcpp/grpcpp.h"
#include "services.grpc.pb.h"

namespace {

constexpr uint64_t kGrpcCallTimeoutMilliseconds = 1000;

class ProcessManagerImpl final : public ProcessManager {
 public:
  explicit ProcessManagerImpl(std::shared_ptr<grpc::Channel> channel,
                              absl::Duration refresh_timeout);

  void SetCallback(
      const std::function<void(std::vector<ProcessInfo>&&)>& listener) override;
  void Start() override;
  void Shutdown() override;

 private:
  void WorkerFunction();

  std::unique_ptr<ProcessService::Stub> process_service_;

  absl::Duration refresh_timeout_;
  absl::Mutex shutdown_mutex_;
  bool shutdown_initiated_;

  absl::Mutex callback_mutex_;
  std::function<void(std::vector<ProcessInfo>&&)> callback_;
  std::thread worker_thread_;
};

ProcessManagerImpl::ProcessManagerImpl(std::shared_ptr<grpc::Channel> channel,
                                       absl::Duration refresh_timeout)
    : process_service_(ProcessService::NewStub(channel)),
      refresh_timeout_(refresh_timeout),
      shutdown_initiated_(false) {}

void ProcessManagerImpl::SetCallback(
    const std::function<void(std::vector<ProcessInfo>&&)>& callback) {
  absl::MutexLock lock(&callback_mutex_);
  callback_ = callback;
}

void ProcessManagerImpl::Start() {
  CHECK(!worker_thread_.joinable());
  worker_thread_ = std::thread([this] { WorkerFunction(); });
}

void ProcessManagerImpl::Shutdown() {
  shutdown_mutex_.Lock();
  shutdown_initiated_ = true;
  shutdown_mutex_.Unlock();
  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }
}

bool IsTrue(bool* var) { return *var; }

void ProcessManagerImpl::WorkerFunction() {
  while (true) {
    if (shutdown_mutex_.LockWhenWithTimeout(
            absl::Condition(IsTrue, &shutdown_initiated_), refresh_timeout_)) {
      // Shutdown was initiated we need to exit
      shutdown_mutex_.Unlock();
      return;
    }
    shutdown_mutex_.Unlock();
    // Timeout expired - refresh the list

    GetProcessListRequest request;
    GetProcessListResponse response;
    grpc::ClientContext context;

    std::chrono::time_point deadline =
        std::chrono::system_clock::now() +
        std::chrono::milliseconds(kGrpcCallTimeoutMilliseconds);
    context.set_deadline(deadline);

    grpc::Status status =
        process_service_->GetProcessList(&context, request, &response);
    if (!status.ok()) {
      ERROR("Grpc call failed: %s", status.error_message());
      continue;
    }

    std::vector<ProcessInfo> processes(response.processes().begin(),
                                       response.processes().end());

    absl::MutexLock callback_lock(&callback_mutex_);
    callback_(std::move(processes));
  }
}

}  // namespace

std::unique_ptr<ProcessManager> ProcessManager::Create(
    std::shared_ptr<grpc::Channel> channel, absl::Duration refresh_timeout) {
  return std::make_unique<ProcessManagerImpl>(channel, refresh_timeout);
}
