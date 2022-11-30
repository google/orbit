// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientServices/ProcessManager.h"

#include <absl/synchronization/mutex.h>

#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "ClientServices/ProcessClient.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_client_services {
namespace {

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;

class ProcessManagerImpl final : public ProcessManager {
 public:
  explicit ProcessManagerImpl(const std::shared_ptr<grpc::Channel>& channel,
                              absl::Duration refresh_timeout);

  ProcessManagerImpl(ProcessManagerImpl&&) = delete;
  ProcessManagerImpl(const ProcessManagerImpl&) = delete;
  ProcessManagerImpl& operator=(ProcessManagerImpl&&) = delete;
  ProcessManagerImpl& operator=(const ProcessManagerImpl&) = delete;

  ~ProcessManagerImpl() override { ShutdownAndWait(); }

  void SetProcessListUpdateListener(
      const std::function<void(std::vector<orbit_grpc_protos::ProcessInfo>)>& listener) override;

  ErrorMessageOr<std::vector<ModuleInfo>> LoadModuleList(uint32_t pid) override;

  ErrorMessageOr<std::string> LoadProcessMemory(uint32_t pid, uint64_t address,
                                                uint64_t size) override;

  ErrorMessageOr<orbit_base::NotFoundOr<std::filesystem::path>> FindDebugInfoFile(
      std::string_view module_path,
      absl::Span<const std::string> additional_search_directories) override;

  void Start();
  void ShutdownAndWait() override;

 private:
  void WorkerFunction();

  std::unique_ptr<ProcessClient> process_client_;

  absl::Duration refresh_timeout_;
  absl::Mutex shutdown_mutex_;
  bool shutdown_initiated_;

  absl::Mutex process_list_update_listener_mutex_;
  std::function<void(std::vector<orbit_grpc_protos::ProcessInfo>)> process_list_update_listener_;

  std::thread worker_thread_;
};

ProcessManagerImpl::ProcessManagerImpl(const std::shared_ptr<grpc::Channel>& channel,
                                       absl::Duration refresh_timeout)
    : refresh_timeout_(refresh_timeout), shutdown_initiated_(false) {
  process_client_ = std::make_unique<ProcessClient>(channel);
}

void ProcessManagerImpl::SetProcessListUpdateListener(
    const std::function<void(std::vector<orbit_grpc_protos::ProcessInfo>)>& listener) {
  absl::MutexLock lock(&process_list_update_listener_mutex_);
  process_list_update_listener_ = listener;
}

ErrorMessageOr<std::vector<ModuleInfo>> ProcessManagerImpl::LoadModuleList(uint32_t pid) {
  return process_client_->LoadModuleList(pid);
}

ErrorMessageOr<orbit_base::NotFoundOr<std::filesystem::path>> ProcessManagerImpl::FindDebugInfoFile(
    std::string_view module_path, absl::Span<const std::string> additional_search_directories) {
  return process_client_->FindDebugInfoFile(module_path, additional_search_directories);
}

void ProcessManagerImpl::Start() {
  ORBIT_CHECK(!worker_thread_.joinable());
  worker_thread_ = std::thread([this] { WorkerFunction(); });
}

void ProcessManagerImpl::ShutdownAndWait() {
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

    // Call a copy of the update listener to allow detaching the listener inside the callback
    std::function<void(std::vector<orbit_grpc_protos::ProcessInfo>)>
        process_list_update_listener_copy;
    {
      absl::MutexLock callback_lock(&process_list_update_listener_mutex_);
      process_list_update_listener_copy = process_list_update_listener_;
    }
    if (process_list_update_listener_copy) {
      process_list_update_listener_copy(std::move(result.value()));
    }
  }
}

ErrorMessageOr<std::string> ProcessManagerImpl::LoadProcessMemory(uint32_t pid, uint64_t address,
                                                                  uint64_t size) {
  return process_client_->LoadProcessMemory(pid, address, size);
}

}  // namespace

std::unique_ptr<ProcessManager> ProcessManager::Create(
    const std::shared_ptr<grpc::Channel>& channel, absl::Duration refresh_timeout) {
  std::unique_ptr<ProcessManagerImpl> impl =
      std::make_unique<ProcessManagerImpl>(channel, refresh_timeout);
  impl->Start();
  return impl;
}

}  // namespace orbit_client_services
