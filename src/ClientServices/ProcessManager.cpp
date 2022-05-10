// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientServices/ProcessManager.h"

#include <absl/container/flat_hash_map.h>
#include <absl/synchronization/mutex.h>
#include <grpcpp/channel.h>

#include <exception>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "ClientServices/LaunchedProcess.h"
#include "ClientServices/ProcessClient.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_client_services {
namespace {

using orbit_client_services::LaunchedProcess;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::ProcessToLaunch;

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

#ifdef _WIN32
  ErrorMessageOr<ProcessInfo> LaunchProcess(const ProcessToLaunch& process_to_launch) override;
  [[nodiscard]] bool IsProcessSpinningAtEntryPoint(uint32_t pid) override;
  [[nodiscard]] bool IsProcessSuspendedAtEntryPoint(uint32_t pid) override;
  void SuspendProcessSpinningAtEntryPoint(uint32_t pid) override;
  void ResumeProcessSuspendedAtEntryPoint(uint32_t pid) override;
#endif

  ErrorMessageOr<std::vector<ModuleInfo>> LoadModuleList(uint32_t pid) override;

  ErrorMessageOr<std::string> LoadProcessMemory(uint32_t pid, uint64_t address,
                                                uint64_t size) override;

  ErrorMessageOr<std::string> LoadNullTerminatedString(uint32_t pid, uint64_t address) override;

  ErrorMessageOr<std::string> FindDebugInfoFile(
      const std::string& module_path,
      absl::Span<const std::string> additional_search_directories) override;

  void Start();
  void ShutdownAndWait();

 private:
  void WorkerFunction();

  std::unique_ptr<ProcessClient> process_client_;

  absl::Duration refresh_timeout_;
  absl::Mutex shutdown_mutex_;
  bool shutdown_initiated_;

  absl::Mutex process_list_update_listener_mutex_;
  std::function<void(std::vector<orbit_grpc_protos::ProcessInfo>)> process_list_update_listener_
      ABSL_GUARDED_BY(process_list_update_listener_mutex_);

  absl::Mutex launched_processes_by_pid_mutex_;
  absl::flat_hash_map<uint32_t, std::unique_ptr<LaunchedProcess>> launched_processes_by_pid_
      ABSL_GUARDED_BY(launched_processes_by_pid_mutex_);

  std::thread worker_thread_;
};

ProcessManagerImpl::ProcessManagerImpl(const std::shared_ptr<grpc::Channel>& channel,
                                       absl::Duration refresh_timeout)
    : refresh_timeout_(refresh_timeout), shutdown_initiated_(false) {
  process_client_ = ProcessClient::Create(channel);
}

void ProcessManagerImpl::SetProcessListUpdateListener(
    const std::function<void(std::vector<orbit_grpc_protos::ProcessInfo>)>& listener) {
  absl::MutexLock lock(&process_list_update_listener_mutex_);
  process_list_update_listener_ = listener;
}

#ifdef _WIN32
ErrorMessageOr<ProcessInfo> ProcessManagerImpl::LaunchProcess(
    const ProcessToLaunch& process_to_launch) {
  OUTCOME_TRY(LaunchedProcess launched_process,
              LaunchedProcess::LaunchProcess(process_to_launch, process_client_.get()));
  absl::MutexLock lock(&launched_processes_by_pid_mutex_);
  const ProcessInfo process_info = launched_process.GetProcessInfo();
  launched_processes_by_pid_.emplace(
      process_info.pid(), std::make_unique<LaunchedProcess>(std::move(launched_process)));
  return process_info;
}

bool ProcessManagerImpl::IsProcessSpinningAtEntryPoint(uint32_t pid) {
  absl::MutexLock lock(&launched_processes_by_pid_mutex_);
  auto it = launched_processes_by_pid_.find(pid);
  return it != launched_processes_by_pid_.end() ? it->second->IsProcessSpinningAtEntryPoint()
                                                : false;
}

bool ProcessManagerImpl::IsProcessSuspendedAtEntryPoint(uint32_t pid) {
  absl::MutexLock lock(&launched_processes_by_pid_mutex_);
  auto it = launched_processes_by_pid_.find(pid);
  return it != launched_processes_by_pid_.end() ? it->second->IsProcessSuspendedAtEntryPoint()
                                                : false;
}

void ProcessManagerImpl::SuspendProcessSpinningAtEntryPoint(uint32_t pid) {
  absl::MutexLock lock(&launched_processes_by_pid_mutex_);
  auto it = launched_processes_by_pid_.find(pid);
  ORBIT_CHECK(it != launched_processes_by_pid_.end());
  ErrorMessageOr<void> result =
      it->second->SuspendProcessSpinningAtEntryPoint(process_client_.get());
  if (result.has_error()) {
    // The process might have been terminated.
    ORBIT_ERROR("Suspending spinning process: %s", result.error().message());
  }
}

void ProcessManagerImpl::ResumeProcessSuspendedAtEntryPoint(uint32_t pid) {
  absl::MutexLock lock(&launched_processes_by_pid_mutex_);
  auto it = launched_processes_by_pid_.find(pid);
  ORBIT_CHECK(it != launched_processes_by_pid_.end());
  ErrorMessageOr<void> result =
      it->second->ResumeProcessSuspendedAtEntryPoint(process_client_.get());
  if (result.has_error()) {
    // The process might have been terminated.
    ORBIT_ERROR("Resuming suspended process: %s", result.error().message());
  }
}
#endif  // _WIN32

ErrorMessageOr<std::vector<ModuleInfo>> ProcessManagerImpl::LoadModuleList(uint32_t pid) {
  return process_client_->LoadModuleList(pid);
}

ErrorMessageOr<std::string> ProcessManagerImpl::FindDebugInfoFile(
    const std::string& module_path, absl::Span<const std::string> additional_search_directories) {
  return process_client_->FindDebugInfoFile(module_path, additional_search_directories);
}

void ProcessManagerImpl::Start() {
  ORBIT_CHECK(!worker_thread_.joinable());
  worker_thread_ = std::thread([this] { WorkerFunction(); });
}

void ProcessManagerImpl::ShutdownAndWait() {
  // Wait for the worker thread to stop, which could take up to refresh_timeout_.
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

ErrorMessageOr<std::string> ProcessManagerImpl::LoadNullTerminatedString(uint32_t pid,
                                                                         uint64_t address) {
  constexpr uint64_t kMaxSize = 256;
  auto error_or_string = LoadProcessMemory(pid, address, kMaxSize);
  if (error_or_string.has_value()) {
    const std::string& str = error_or_string.value();
    if (str.find('\0') == std::string::npos) {
      const char* error_msg = "Remote string is not null terminated";
      ORBIT_ERROR("%s: %s", error_msg, str.c_str());
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

}  // namespace orbit_client_services
