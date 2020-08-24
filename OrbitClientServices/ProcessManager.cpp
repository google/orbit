// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientServices/ProcessManager.h"

#include <chrono>
#include <memory>
#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitClientServices/ProcessClient.h"
#include "grpcpp/grpcpp.h"
#include "outcome.hpp"
#include "symbol.pb.h"

namespace {

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::TracepointInfo;

class ProcessManagerImpl final : public ProcessManager {
 public:
  explicit ProcessManagerImpl(const std::shared_ptr<grpc::Channel>& channel,
                              absl::Duration refresh_timeout);

  void SetProcessListUpdateListener(const std::function<void(ProcessManager*)>& listener) override;

  std::vector<ProcessInfo> GetProcessList() const override;
  ErrorMessageOr<std::vector<ModuleInfo>> LoadModuleList(int32_t pid) override;

  ErrorMessageOr<std::vector<TracepointInfo>> LoadTracepointList() override;

  ErrorMessageOr<std::string> LoadProcessMemory(int32_t pid, uint64_t address,
                                                uint64_t size) override;

  ErrorMessageOr<std::string> LoadNullTerminatedString(int32_t pid, uint64_t address) override;

  ErrorMessageOr<std::string> FindDebugInfoFile(const std::string& module_path) override;

  void Start();
  void Shutdown() override;

 private:
  void WorkerFunction();

  std::unique_ptr<ProcessClient> process_client_;

  absl::Duration refresh_timeout_;
  absl::Mutex shutdown_mutex_;
  bool shutdown_initiated_;

  mutable absl::Mutex mutex_;
  std::vector<ProcessInfo> process_list_;
  std::function<void(ProcessManager*)> process_list_update_listener_;

  std::thread worker_thread_;
};

ProcessManagerImpl::ProcessManagerImpl(const std::shared_ptr<grpc::Channel>& channel,
                                       absl::Duration refresh_timeout)
    : refresh_timeout_(refresh_timeout), shutdown_initiated_(false) {
  process_client_ = std::make_unique<ProcessClient>(channel);
}

void ProcessManagerImpl::SetProcessListUpdateListener(
    const std::function<void(ProcessManager*)>& listener) {
  absl::MutexLock lock(&mutex_);
  process_list_update_listener_ = listener;
}

ErrorMessageOr<std::vector<ModuleInfo>> ProcessManagerImpl::LoadModuleList(int32_t pid) {
  return process_client_->LoadModuleList(pid);
}

ErrorMessageOr<std::vector<TracepointInfo>> ProcessManagerImpl::LoadTracepointList() {
  return process_client_->LoadTracepointList();
}

std::vector<ProcessInfo> ProcessManagerImpl::GetProcessList() const {
  absl::MutexLock lock(&mutex_);
  return process_list_;
}

ErrorMessageOr<std::string> ProcessManagerImpl::FindDebugInfoFile(const std::string& module_path) {
  return process_client_->FindDebugInfoFile(module_path);
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
    if (shutdown_mutex_.LockWhenWithTimeout(absl::Condition(IsTrue, &shutdown_initiated_),
                                            refresh_timeout_)) {
      // Shutdown was initiated we need to exit
      shutdown_mutex_.Unlock();
      return;
    }
    shutdown_mutex_.Unlock();
    // Timeout expired - refresh the list

    ErrorMessageOr<std::vector<ProcessInfo>> result = process_client_->GetProcessList();
    if (result.has_error()) {
      continue;
    }

    absl::MutexLock callback_lock(&mutex_);
    const auto& processes = result.value();
    process_list_.assign(processes.begin(), processes.end());
    if (process_list_update_listener_) {
      process_list_update_listener_(this);
    }
  }
}

ErrorMessageOr<std::string> ProcessManagerImpl::LoadProcessMemory(int32_t pid, uint64_t address,
                                                                  uint64_t size) {
  return process_client_->LoadProcessMemory(pid, address, size);
}

ErrorMessageOr<std::string> ProcessManagerImpl::LoadNullTerminatedString(int32_t pid,
                                                                         uint64_t address) {
  constexpr uint64_t max_size = 256;
  auto error_or_string = LoadProcessMemory(pid, address, max_size);
  if (error_or_string.has_value()) {
    const std::string& str = error_or_string.value();
    if (str.find('\0') == std::string::npos) {
      const char* error_msg = "Remote string is not null terminated";
      ERROR("%s: %s", error_msg, str.c_str());
      return ErrorMessage(error_msg);
    }

    // The string has a size of max_size at this point. Shrink it by assigning
    // it its own str.c_str(). c_str() is guaranteed to be null terminated.
    error_or_string = str.c_str();
  }

  return error_or_string;
}

}  // namespace

std::unique_ptr<ProcessManager> ProcessManager::Create(
    const std::shared_ptr<grpc::Channel>& channel, absl::Duration refresh_timeout) {
  std::unique_ptr<ProcessManagerImpl> impl =
      std::make_unique<ProcessManagerImpl>(channel, refresh_timeout);
  impl->Start();
  return impl;
}
