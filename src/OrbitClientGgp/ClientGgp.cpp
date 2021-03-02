// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientGgp/ClientGgp.h"

#include <absl/flags/declare.h>
#include <absl/strings/str_format.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/channel_arguments.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <type_traits>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitCaptureClient/CaptureClient.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "OrbitClientData/FunctionUtils.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientData/UserDefinedCaptureData.h"
#include "OrbitClientModel/CaptureSerializer.h"
#include "OrbitClientModel/SamplingDataPostProcessor.h"
#include "StringManager.h"
#include "SymbolHelper.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/flags/flag.h"
#include "capture_data.pb.h"
#include "module.pb.h"
#include "process.pb.h"

ABSL_DECLARE_FLAG(bool, thread_state);
ABSL_DECLARE_FLAG(uint64_t, max_local_marker_depth_per_command_buffer);

using orbit_base::Future;
using orbit_client_protos::CallstackEvent;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::TimerInfo;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;

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
  string_manager_ = std::make_shared<StringManager>();

  return true;
}

// Client requests to start the capture
bool ClientGgp::RequestStartCapture(ThreadPool* thread_pool) {
  int32_t pid = target_process_.pid();
  if (pid == -1) {
    ERROR(
        "Error starting capture: "
        "No process selected. Please choose a target process for the capture.");
    return false;
  }

  ClearCapture();

  LOG("Capture pid %d", pid);
  TracepointInfoSet selected_tracepoints;
  bool collect_thread_state = absl::GetFlag(FLAGS_thread_state);
  bool bulk_capture_events = false;
  uint64_t max_local_marker_depth_per_command_buffer =
      absl::GetFlag(FLAGS_max_local_marker_depth_per_command_buffer);
  bool enable_introspection = false;
  Future<ErrorMessageOr<CaptureOutcome>> result = capture_client_->Capture(
      thread_pool, target_process_, module_manager_, selected_functions_, selected_tracepoints,
      absl::flat_hash_set<uint64_t>{}, collect_thread_state, bulk_capture_events,
      enable_introspection, max_local_marker_depth_per_command_buffer);

  orbit_base::ImmediateExecutor executer;
  result.Then(&executer, [this](ErrorMessageOr<CaptureOutcome> result) {
    if (!result.has_value()) {
      ClearCapture();
      ERROR("Capture failed: %s", result.error().message());
      return;
    }

    switch (result.value()) {
      case CaptureListener::CaptureOutcome::kCancelled:
        ClearCapture();
        return;
      case CaptureListener::CaptureOutcome::kComplete:
        PostprocessCaptureData();
        return;
    }
  });

  return true;
}

bool ClientGgp::StopCapture() {
  LOG("Request to stop capture");
  return capture_client_->StopCapture();
}

bool ClientGgp::SaveCapture() {
  LOG("Saving capture");
  const auto& key_to_string_map = string_manager_->GetKeyToStringMap();
  std::string file_name = options_.capture_file_name;
  if (file_name.empty()) {
    file_name = capture_serializer::GetCaptureFileName(GetCaptureData());
  } else {
    // Make sure the file is saved with orbit extension
    capture_serializer::IncludeOrbitExtensionInFile(file_name);
  }
  // Add the location where the capture is saved
  file_name.insert(0, options_.capture_file_directory);

  ErrorMessageOr<void> result = capture_serializer::Save(
      file_name, GetCaptureData(), key_to_string_map, timer_infos_.begin(), timer_infos_.end());
  if (result.has_error()) {
    ERROR("Could not save the capture: %s", result.error().message());
    return false;
  }
  return true;
}

ErrorMessageOr<ProcessData> ClientGgp::GetOrbitProcessByPid(int32_t pid) {
  // We retrieve the information of the process to later get the module corresponding to its binary
  OUTCOME_TRY(process_infos, process_client_->GetProcessList());
  LOG("List of processes:");
  for (const ProcessInfo& info : process_infos) {
    LOG("pid:%d, name:%s, path:%s, is64:%d", info.pid(), info.name(), info.full_path(),
        info.is_64_bit());
  }
  auto process_it = find_if(process_infos.begin(), process_infos.end(),
                            [&pid](const ProcessInfo& info) { return info.pid() == pid; });
  if (process_it == process_infos.end()) {
    return ErrorMessage(absl::StrFormat("Error: Process with pid %d not found", pid));
  }
  LOG("Found process by pid, set target process");
  ProcessData process(*process_it);
  LOG("Process info: pid:%d, name:%s, path:%s, is64:%d", process.pid(), process.name(),
      process.full_path(), process.is_64_bit());
  return std::move(process);
}

ErrorMessageOr<void> ClientGgp::LoadModuleAndSymbols() {
  // Load modules for target_process_
  OUTCOME_TRY(module_infos, process_client_->LoadModuleList(target_process_.pid()));

  LOG("List of modules");
  for (const ModuleInfo& info : module_infos) {
    LOG("name:%s, path:%s, size:%d, address_start:%d. address_end:%d, build_id:%s", info.name(),
        info.file_path(), info.file_size(), info.address_start(), info.address_end(),
        info.build_id());
  }

  module_manager_.AddOrUpdateModules(module_infos);

  // Process name can be arbitrary so we use the path to find the module corresponding to the binary
  // of target_process_
  main_module_ = module_manager_.GetMutableModuleByPath(target_process_.full_path());
  if (main_module_ == nullptr) {
    return ErrorMessage("Error: Module corresponding to process binary not found");
  }
  LOG("Found module correspondent to process binary");
  LOG("Module info: name:%s, path:%s, size:%d, build_id:%s", main_module_->name(),
      main_module_->file_path(), main_module_->file_size(), main_module_->build_id());

  target_process_.UpdateModuleInfos(module_infos);

  // Load symbols for the module
  const std::string& module_path = main_module_->file_path();
  LOG("Looking for debug info file for %s", module_path);
  OUTCOME_TRY(main_executable_debug_file, process_client_->FindDebugInfoFile(module_path));
  LOG("Found file: %s", main_executable_debug_file);
  LOG("Loading symbols");
  OUTCOME_TRY(symbols, SymbolHelper::LoadSymbolsFromFile(main_executable_debug_file));
  main_module_->AddSymbols(symbols);
  return outcome::success();
}

bool ClientGgp::InitCapture() {
  ErrorMessageOr<ProcessData> target_process_result = GetOrbitProcessByPid(options_.capture_pid);
  if (target_process_result.has_error()) {
    ERROR("Not able to set target process: %s", target_process_result.error().message());
    return false;
  }
  target_process_ = std::move(target_process_result.value());
  // Load the module and symbols
  ErrorMessageOr<void> result = LoadModuleAndSymbols();
  if (result.has_error()) {
    ERROR("Not possible to finish loading the module and symbols: %s", result.error().message());
    return false;
  }
  // Load selected functions
  LoadSelectedFunctions();
  return true;
}

void ClientGgp::LoadSelectedFunctions() {
  // Load selected functions if provided
  if (!options_.capture_functions.empty()) {
    LOG("Loading selected functions");
    selected_functions_ = GetSelectedFunctions();
    if (!selected_functions_.empty()) {
      LOG("List of selected functions to hook in the capture:");
      for (auto const& [address, selected_function] : selected_functions_) {
        LOG("%d %s", address, selected_function.pretty_name());
      }
    }
  } else {
    LOG("No functions provided; no functions hooked in the capture");
  }
}

void ClientGgp::InformUsedSelectedCaptureFunctions(
    const absl::flat_hash_set<std::string>& capture_functions_used) {
  if (capture_functions_used.size() != options_.capture_functions.size()) {
    for (const std::string& selected_function : options_.capture_functions) {
      if (!capture_functions_used.contains(selected_function)) {
        ERROR("Function matching %s not found; will not be hooked in the capture",
              selected_function);
      }
    }
  } else {
    LOG("All functions provided had at least a match");
  }
}

std::string ClientGgp::SelectedFunctionMatch(const FunctionInfo& func) {
  for (const std::string& selected_function : options_.capture_functions) {
    if (func.pretty_name().find(selected_function) != std::string::npos) {
      return selected_function;
    }
  }
  return {};
}

absl::flat_hash_map<uint64_t, FunctionInfo> ClientGgp::GetSelectedFunctions() {
  uint64_t function_id = 1;
  absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions;
  absl::flat_hash_set<std::string> capture_functions_used;
  for (const FunctionInfo* func : main_module_->GetFunctions()) {
    const std::string& selected_function_match = SelectedFunctionMatch(*func);
    if (!selected_function_match.empty()) {
      selected_functions[function_id++] = *func;
      if (!capture_functions_used.contains(selected_function_match)) {
        capture_functions_used.insert(selected_function_match);
      }
    }
  }
  InformUsedSelectedCaptureFunctions(capture_functions_used);
  return selected_functions;
}

void ClientGgp::UpdateCaptureFunctions(std::vector<std::string> capture_functions) {
  options_.capture_functions = std::move(capture_functions);
  LoadSelectedFunctions();
}

void ClientGgp::ClearCapture() {
  capture_data_.reset();
  string_manager_->Clear();
  timer_infos_.clear();
}

void ClientGgp::ProcessTimer(const TimerInfo& timer_info) { timer_infos_.push_back(timer_info); }

// CaptureListener implementation
void ClientGgp::OnCaptureStarted(
    ProcessData&& process,
    absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions,
    TracepointInfoSet selected_tracepoints,
    absl::flat_hash_set<uint64_t> frame_track_function_ids) {
  capture_data_ = CaptureData(std::move(process), &module_manager_, std::move(selected_functions),
                              std::move(selected_tracepoints), std::move(frame_track_function_ids));
  LOG("Capture started");
}

void ClientGgp::PostprocessCaptureData() {
  LOG("Capture completed");
  GetMutableCaptureData().FilterBrokenCallstacks();
  GetMutableCaptureData().set_post_processed_sampling_data(
      orbit_client_model::CreatePostProcessedSamplingData(*GetCaptureData().GetCallstackData(),
                                                          GetCaptureData()));
}

void ClientGgp::OnTimer(const orbit_client_protos::TimerInfo& timer_info) {
  if (timer_info.function_id() != 0) {
    const FunctionInfo* func =
        GetCaptureData().GetInstrumentedFunctionById(timer_info.function_id());
    // For timers, the function must be present in the process
    CHECK(func != nullptr);
    uint64_t elapsed_nanos = timer_info.end() - timer_info.start();
    GetMutableCaptureData().UpdateFunctionStats(*func, elapsed_nanos);
  }
  ProcessTimer(timer_info);
}

void ClientGgp::OnKeyAndString(uint64_t key, std::string str) {
  string_manager_->AddIfNotPresent(key, std::move(str));
}

void ClientGgp::OnUniqueCallStack(CallStack callstack) {
  GetMutableCaptureData().AddUniqueCallStack(std::move(callstack));
}

void ClientGgp::OnCallstackEvent(CallstackEvent callstack_event) {
  GetMutableCaptureData().AddCallstackEvent(std::move(callstack_event));
}

void ClientGgp::OnThreadName(int32_t thread_id, std::string thread_name) {
  GetMutableCaptureData().AddOrAssignThreadName(thread_id, std::move(thread_name));
}

void ClientGgp::OnThreadStateSlice(orbit_client_protos::ThreadStateSliceInfo thread_state_slice) {
  GetMutableCaptureData().AddThreadStateSlice(std::move(thread_state_slice));
}

void ClientGgp::OnAddressInfo(LinuxAddressInfo address_info) {
  GetMutableCaptureData().InsertAddressInfo(std::move(address_info));
}

void ClientGgp::OnUniqueTracepointInfo(uint64_t key,
                                       orbit_grpc_protos::TracepointInfo tracepoint_info) {
  GetMutableCaptureData().AddUniqueTracepointEventInfo(key, std::move(tracepoint_info));
}

void ClientGgp::OnTracepointEvent(orbit_client_protos::TracepointEventInfo tracepoint_event_info) {
  int32_t capture_process_id = GetCaptureData().process_id();
  bool is_same_pid_as_target = capture_process_id == tracepoint_event_info.pid();

  GetMutableCaptureData().AddTracepointEventAndMapToThreads(
      tracepoint_event_info.time(), tracepoint_event_info.tracepoint_info_key(),
      tracepoint_event_info.pid(), tracepoint_event_info.tid(), tracepoint_event_info.cpu(),
      is_same_pid_as_target);
}
