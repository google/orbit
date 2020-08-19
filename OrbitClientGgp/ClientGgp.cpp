// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientGgp.h"

#include <condition_variable>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <thread>

#include "Capture.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitCaptureClient/CaptureClient.h"
#include "OrbitClientServices/ProcessManager.h"
#include "absl/time/time.h"
#include "capture_data.pb.h"

using orbit_grpc_protos::ProcessInfo;

ClientGgp::ClientGgp(ClientGgpOptions&& options) : options_(std::move(options)) {}

bool ClientGgp::InitClient() {
  if (options_.grpc_server_address.empty()) {
    ERROR("gRPC server address not provided");
    return false;
  }

  // Create channel
  grpc::ChannelArguments channel_arguments;
  channel_arguments.SetMaxReceiveMessageSize(std::numeric_limits<int32_t>::max());

  grpc_channel_ = grpc::CreateCustomChannel(options_.grpc_server_address,
                                            grpc::InsecureChannelCredentials(), channel_arguments);
  if (!grpc_channel_) {
    ERROR("Unable to create GRPC channel to %s", options_.grpc_server_address);
    return false;
  }
  LOG("Created GRPC channel to %s", options_.grpc_server_address);

  process_manager_ = ProcessManager::Create(grpc_channel_, options_.process_refresh_timeout);
  auto callback = [this](ProcessManager*) { process_manager_ready_.notify_one(); };
  process_manager_->SetProcessListUpdateListener(callback);
  // wait for the process list to be updated once
  std::unique_lock<std::mutex> lk(mutex_);
  process_manager_ready_.wait(lk);
  process_manager_->SetProcessListUpdateListener(nullptr);

  // Initialisations needed for capture to work
  if (!InitCapture()) {
    ShutdownClient();
    return false;
  }
  capture_client_ = std::make_unique<CaptureClient>(grpc_channel_, this);
  return true;
}

void ClientGgp::ShutdownClient() { process_manager_->Shutdown(); }

// Client requests to start the capture
bool ClientGgp::RequestStartCapture(ThreadPool* thread_pool) {
  int32_t pid = target_process_->GetId();
  if (pid == -1) {
    ERROR(
        "Error starting capture: "
        "No process selected. Please choose a target process for the capture.");
    return false;
  }
  LOG("Capture pid %d", pid);

  // TODO: selected_functions available when UploadSymbols is included
  // TODO(kuebler): right now selected_functions is only an empty placeholder,
  //  it needs to be filled separately in each client and then passed.
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions;

  ErrorMessageOr<void> result = capture_client_->StartCapture(thread_pool, pid, selected_functions);
  if (result.has_error()) {
    ERROR("Error starting capture: %s", result.error().message());
    return false;
  }
  return true;
}

bool ClientGgp::StopCapture() {
  LOG("Request to stop capture");
  return capture_client_->StopCapture();
}

std::shared_ptr<Process> ClientGgp::GetOrbitProcessByPid(int32_t pid) {
  // We retrieve the information of the process to later get the module corresponding to its binary
  const std::vector<ProcessInfo>& process_infos = process_manager_->GetProcessList();
  LOG("List of processes:");
  for (const ProcessInfo& info : process_infos) {
    LOG("pid:%d, name:%s, path:%s, is64:%d", info.pid(), info.name(), info.full_path(),
        info.is_64_bit());
  }
  auto process_it = find_if(process_infos.begin(), process_infos.end(),
                            [&pid](const ProcessInfo& info) { return info.pid() == pid; });
  if (process_it != process_infos.end()) {
    LOG("Found process by pid, set target process");
    std::shared_ptr<Process> process = std::make_shared<Process>();
    process->SetID(process_it->pid());
    process->SetName(process_it->name());
    process->SetFullPath(process_it->full_path());
    process->SetIs64Bit(process_it->is_64_bit());
    LOG("Process info: pid:%d, name:%s, path:%s, is64:%d", process->GetID(), process->GetName(),
        process->GetFullPath(), process->GetIs64Bit());
    return process;
  }
  return nullptr;
}

bool ClientGgp::InitCapture() {
  target_process_ = GetOrbitProcessByPid(options_.capture_pid);
  if (target_process_ == nullptr) {
    ERROR("Error: not able to set target process");
    return false;
  }
  // TODO: remove this line when GTargetProcess is deprecated in Capture
  Capture::SetTargetProcess(target_process_);
  return true;
}

// CaptureListener implementation
void ClientGgp::OnCaptureStarted(
    int32_t process_id,
    const absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo>& selected_functions) {
  Capture::capture_data_ =
      CaptureData(process_id, target_process_->GetName(), target_process_, selected_functions);
  LOG("Capture started");
}

void ClientGgp::OnCaptureComplete() { LOG("Capture completed"); }

void ClientGgp::OnTimer(const orbit_client_protos::TimerInfo&) {}

void ClientGgp::OnKeyAndString(uint64_t, std::string) {}

void ClientGgp::OnUniqueCallStack(CallStack) {}

void ClientGgp::OnCallstackEvent(orbit_client_protos::CallstackEvent) {}

void ClientGgp::OnThreadName(int32_t, std::string) {}

void ClientGgp::OnAddressInfo(orbit_client_protos::LinuxAddressInfo) {}