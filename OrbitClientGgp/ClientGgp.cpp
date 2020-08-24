// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientGgp.h"

#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <string>

#include "Capture.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitCaptureClient/CaptureClient.h"
#include "OrbitClientServices/ProcessManager.h"
#include "capture_data.pb.h"

using orbit_grpc_protos::ModuleInfo;
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

  process_client_ = std::make_unique<ProcessClient>(grpc_channel_);

  // Initialisations needed for capture to work
  if (!InitCapture()) {
    return false;
  }
  capture_client_ = std::make_unique<CaptureClient>(grpc_channel_, this);
  return true;
}

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

  // TODO: right now selected_functions is only an empty placeholder,
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
  ErrorMessageOr<std::vector<ProcessInfo>> result_process_infos = process_client_->GetProcessList();
  if (result_process_infos.has_error()) {
    return nullptr;
  }
  const std::vector<ProcessInfo>& process_infos = result_process_infos.value();
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
    LOG("Process info: pid:%d, name:%s, path:%s, is64:%d", process->GetId(), process->GetName(),
        process->GetFullPath(), process->GetIs64Bit());
    return process;
  }
  return nullptr;
}

bool ClientGgp::LoadModuleAndSymbols() {
  // Load modules for target_process_
  ErrorMessageOr<std::vector<ModuleInfo>> result_module_infos =
      process_client_->LoadModuleList(target_process_->GetId());
  if (result_module_infos.has_error()) {
    ERROR("Error retrieving modules: %s", result_module_infos.error().message());
    return false;
  }
  // Find module that corresponds to the binary of target_process
  const std::vector<ModuleInfo>& module_infos = result_module_infos.value();
  LOG("List of modules");
  for (const ModuleInfo& info : module_infos) {
    LOG("name:%s, path:%s, size:%d, address_start:%d. address_end:%d, build_id:%s", info.name(),
        info.file_path(), info.file_size(), info.address_start(), info.address_end(),
        info.build_id());
  }
  std::string_view target_name = target_process_->GetName();
  auto module_it =
      find_if(module_infos.begin(), module_infos.end(),
              [&target_name](const ModuleInfo& info) { return info.name() == target_name; });
  if (module_it == module_infos.end()) {
    ERROR("Error: Module correspondent to process binary not found");
    return false;
  }
  LOG("Found module correspondent to process binary");
  std::shared_ptr<Module> module = std::make_shared<Module>();
  module->m_Name = module_it->name();
  module->m_FullName = module_it->file_path();
  module->m_PdbSize = module_it->file_size();
  module->m_AddressStart = module_it->address_start();
  module->m_AddressEnd = module_it->address_end();
  module->m_DebugSignature = module_it->build_id();
  target_process_->AddModule(module);
  LOG("Module info: name:%s, path:%s, size:%d, address_start:%d. address_end:%d, build_id:%s",
      module->m_Name, module->m_FullName, module->m_PdbSize, module->m_AddressStart,
      module->m_AddressEnd, module->m_DebugSignature);

  // Load symbols for the module
  const std::string& module_path = module->m_FullName;
  LOG("Looking for debug info file for %s", module_path);
  ErrorMessageOr<std::string> result_debug_file = process_client_->FindDebugInfoFile(module_path);
  if (result_debug_file.has_error()) {
    ERROR("Error loading symbols: %s", result_debug_file.error().message());
    return false;
  }
  const std::string& debug_file_path = result_debug_file.value();
  LOG("Found file: %s", debug_file_path);
  LOG("Loading symbols");
  ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> result_symbols =
      symbol_helper_.LoadSymbolsFromFile(debug_file_path, module->m_DebugSignature);
  if (result_symbols.has_error()) {
    ERROR("Error loading symbols: %s", result_symbols.error().message());
    return false;
  }
  module->LoadSymbols(result_symbols.value());
  target_process_->AddFunctions(module->m_Pdb->GetFunctions());
  return true;
}

bool ClientGgp::InitCapture() {
  target_process_ = GetOrbitProcessByPid(options_.capture_pid);
  if (target_process_ == nullptr) {
    ERROR("Error: not able to set target process");
    return false;
  }
  // Load the module and symbols
  if (!LoadModuleAndSymbols()) {
    ERROR("Error: not possible to finish loading the module and symbols");
    return false;
  }
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