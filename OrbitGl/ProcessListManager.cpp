// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessListManager.h"

#include <memory>
#include <string>

#include "OrbitBase/Logging.h"
#include "grpcpp/grpcpp.h"
#include "services.grpc.pb.h"

namespace {

class ProcessListManagerImpl final : public ProcessListManager {
 public:
  explicit ProcessListManagerImpl(std::shared_ptr<grpc::Channel> channel,
                                  absl::Duration refresh_timeout);

  void SetCallback(const std::function<void(std::vector<ProcessInfo>&&)>&
                       listener) override;
  void Start() override;
  void Shutdown() override;
  void Wait() override;

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

ProcessListManagerImpl::ProcessListManagerImpl(
    std::shared_ptr<grpc::Channel> channel, absl::Duration refresh_timeout)
    : process_service_(ProcessService::NewStub(channel)),
      refresh_timeout_(refresh_timeout),
      shutdown_initiated_(false) {}

void ProcessListManagerImpl::SetCallback(
    const std::function<void(std::vector<ProcessInfo>&&)>& callback) {
  absl::MutexLock lock(&callback_mutex_);
  callback_ = callback;
}

void ProcessListManagerImpl::Start() {
  CHECK(!worker_thread_.joinable());
  worker_thread_ = std::thread([this] { WorkerFunction(); });
}

void ProcessListManagerImpl::Shutdown() {
  absl::MutexLock lock(&shutdown_mutex_);
  shutdown_initiated_ = true;
}

void ProcessListManagerImpl::Wait() {
  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }
}

bool IsTrue(bool* var) { return *var; }

void ProcessListManagerImpl::WorkerFunction() {
  while (true) {
    if (shutdown_mutex_.LockWhenWithTimeout(
            absl::Condition(IsTrue, &shutdown_initiated_), refresh_timeout_)) {
      // Shutdown was initiated we need to exit
      return;
    }
    shutdown_mutex_.Unlock();

    GetProcessListRequest request;
    GetProcessListReply reply;
    grpc::ClientContext context;

    // Timeout expired - refresh the list
    grpc::Status status =
        process_service_->GetProcessList(&context, request, &reply);
    if (!status.ok()) {
      ERROR("Grpc call failed: %s", status.error_message());
      continue;
    }

    std::vector<ProcessInfo> processes(reply.processes().begin(),
                                       reply.processes().end());

    absl::MutexLock callback_lock(&callback_mutex_);
    callback_(std::move(processes));
  }
}

}  // namespace

std::unique_ptr<ProcessListManager> ProcessListManager::Create(
    std::shared_ptr<grpc::Channel> channel, absl::Duration refresh_timeout) {
  return std::make_unique<ProcessListManagerImpl>(channel, refresh_timeout);
}
