// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientGgp/ClientGgp.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/time/clock.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/channel_arguments.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "CaptureClient/CaptureClient.h"
#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureClient/CaptureListener.h"
#include "CaptureClient/ClientCaptureOptions.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ProcessData.h"
#include "ClientData/TracepointCustom.h"
#include "ClientModel/CaptureSerializer.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "ObjectUtils/SymbolsFile.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadConstants.h"
#include "Symbols/SymbolHelper.h"

ABSL_DECLARE_FLAG(bool, thread_state);
ABSL_DECLARE_FLAG(uint64_t, max_local_marker_depth_per_command_buffer);

using orbit_base::Future;

using orbit_capture_client::CaptureClient;
using orbit_capture_client::CaptureEventProcessor;
using orbit_capture_client::CaptureListener;
using orbit_capture_client::ClientCaptureOptions;

using orbit_client_data::FunctionInfo;
using orbit_client_data::ProcessData;
using orbit_client_data::TracepointInfoSet;

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;

using DynamicInstrumentationMethod =
    orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod;
using UnwindingMethod = orbit_grpc_protos::CaptureOptions::UnwindingMethod;

bool ClientGgp::InitClient() {
  if (options_.grpc_server_address.empty()) {
    ORBIT_ERROR("gRPC server address not provided");
    return false;
  }

  // Create channel
  grpc::ChannelArguments channel_arguments;
  channel_arguments.SetMaxReceiveMessageSize(std::numeric_limits<int32_t>::max());

  grpc_channel_ = grpc::CreateCustomChannel(options_.grpc_server_address,
                                            grpc::InsecureChannelCredentials(), channel_arguments);
  if (!grpc_channel_) {
    ORBIT_ERROR("Unable to create GRPC channel to %s", options_.grpc_server_address);
    return false;
  }
  ORBIT_LOG("Created GRPC channel to %s", options_.grpc_server_address);

  process_client_ = std::make_unique<orbit_client_services::ProcessClient>(grpc_channel_);

  // Initialisations needed for capture to work
  if (!InitCapture()) {
    return false;
  }
  capture_client_ = std::make_unique<CaptureClient>(grpc_channel_);

  return true;
}

// Client requests to start the capture
ErrorMessageOr<void> ClientGgp::RequestStartCapture(orbit_base::ThreadPool* thread_pool) {
  uint32_t pid = target_process_->pid();
  if (pid == orbit_base::kInvalidThreadId) {
    return ErrorMessage{
        "Error starting capture: "
        "No process selected. Please choose a target process for the capture."};
  }

  ORBIT_LOG("Capture pid %d", pid);
  ClientCaptureOptions options;
  options.process_id = pid;
  options.unwinding_method =
      options_.use_framepointer_unwinding ? CaptureOptions::kFramePointers : CaptureOptions::kDwarf;
  options.collect_scheduling_info = true;
  options.collect_thread_states = absl::GetFlag(FLAGS_thread_state);
  options.collect_gpu_jobs = true;
  options.enable_api = false;
  options.enable_introspection = false;
  options.dynamic_instrumentation_method = CaptureOptions::kKernelUprobes;
  options.max_local_marker_depth_per_command_buffer =
      absl::GetFlag(FLAGS_max_local_marker_depth_per_command_buffer);
  options.selected_functions = selected_functions_;
  options.stack_dump_size = options_.stack_dump_size;
  options.samples_per_second = options_.samples_per_second;

  std::filesystem::path file_path = GenerateFilePath();

  ORBIT_LOG("Saving capture to \"%s\"", file_path.string());

  OUTCOME_TRY(auto&& event_processor, CaptureEventProcessor::CreateSaveToFileProcessor(
                                          GenerateFilePath(), [](const ErrorMessage& error) {
                                            ORBIT_ERROR("%s", error.message());
                                          }));

  Future<ErrorMessageOr<CaptureListener::CaptureOutcome>> result = capture_client_->Capture(
      thread_pool, std::move(event_processor), module_manager_, *target_process_, options);

  orbit_base::ImmediateExecutor executor;

  result.Then(&executor, [](ErrorMessageOr<CaptureListener::CaptureOutcome> result) {
    if (!result.has_value()) {
      ORBIT_ERROR("Capture failed: %s", result.error().message());
    }

    // We do not send try aborting capture - it cannot be cancelled.
    ORBIT_CHECK(result.value() == CaptureListener::CaptureOutcome::kComplete);
  });

  return outcome::success();
}

bool ClientGgp::StopCapture() {
  ORBIT_LOG("Request to stop capture");
  return capture_client_->StopCapture();
}

std::filesystem::path ClientGgp::GenerateFilePath() {
  std::string file_name = options_.capture_file_name;
  if (file_name.empty()) {
    file_name = orbit_client_model::capture_serializer::GenerateCaptureFileName(
        target_process_->name(), absl::Now());
  } else {
    // Make sure the file is saved with orbit extension
    orbit_client_model::capture_serializer::IncludeOrbitExtensionInFile(file_name);
  }

  // Add the location where the capture is saved
  return std::filesystem::path(options_.capture_file_directory) / file_name;
}

ErrorMessageOr<std::unique_ptr<ProcessData>> ClientGgp::GetOrbitProcessByPid(uint32_t pid) {
  // We retrieve the information of the process to later get the module corresponding to its binary
  OUTCOME_TRY(auto&& process_infos, process_client_->GetProcessList());
  ORBIT_LOG("List of processes:");
  for (const ProcessInfo& info : process_infos) {
    ORBIT_LOG("pid:%d, name:%s, path:%s, is64:%d", info.pid(), info.name(), info.full_path(),
              info.is_64_bit());
  }
  auto process_it = find_if(process_infos.begin(), process_infos.end(),
                            [&pid](const ProcessInfo& info) { return info.pid() == pid; });
  if (process_it == process_infos.end()) {
    return ErrorMessage(absl::StrFormat("Error: Process with pid %d not found", pid));
  }
  ORBIT_LOG("Found process by pid, set target process");
  auto process = std::make_unique<ProcessData>(*process_it, &module_identifier_provider_);
  ORBIT_LOG("Process info: pid:%d, name:%s, path:%s, is64:%d", process->pid(), process->name(),
            process->full_path(), process->is_64_bit());
  return process;
}

ErrorMessageOr<void> ClientGgp::LoadModuleAndSymbols() {
  // Load modules for target_process_
  OUTCOME_TRY(auto&& module_infos, process_client_->LoadModuleList(target_process_->pid()));

  ORBIT_LOG("List of modules");
  std::string target_process_build_id;
  for (const ModuleInfo& info : module_infos) {
    // TODO(dimitry): eventually we want to get build_id directly from ProcessData
    if (info.file_path() == target_process_->full_path()) {
      target_process_build_id = info.build_id();
    }
    ORBIT_LOG("name:%s, path:%s, size:%d, address_start:%d. address_end:%d, build_id:%s",
              info.name(), info.file_path(), info.file_size(), info.address_start(),
              info.address_end(), info.build_id());
  }

  ORBIT_CHECK(module_manager_.AddOrUpdateModules(module_infos).empty());

  // Process name can be arbitrary so we use the path to find the module corresponding to the binary
  // of target_process_
  main_module_ = module_manager_.GetMutableModuleByModulePathAndBuildId(
      {.module_path = target_process_->full_path(), .build_id = target_process_build_id});
  if (main_module_ == nullptr) {
    return ErrorMessage("Error: Module corresponding to process binary not found");
  }
  ORBIT_LOG("Found module correspondent to process binary");
  ORBIT_LOG("Module info: name:%s, path:%s, size:%d, build_id:%s", main_module_->name(),
            main_module_->file_path(), main_module_->file_size(), main_module_->build_id());

  target_process_->UpdateModuleInfos(module_infos);

  // Load symbols for the module
  const std::string& module_path = main_module_->file_path();
  ORBIT_LOG("Looking for debug info file for %s", module_path);
  OUTCOME_TRY(orbit_base::NotFoundOr<std::filesystem::path> && find_result,
              process_client_->FindDebugInfoFile(module_path, {}));
  if (orbit_base::IsNotFound(find_result)) {
    return ErrorMessage{
        absl::StrFormat("Symbols not found: %s", orbit_base::GetNotFoundMessage(find_result))};
  }
  const auto& main_executable_debug_file{orbit_base::GetFound(find_result)};
  ORBIT_LOG("Found file: %s", main_executable_debug_file);
  ORBIT_LOG("Loading symbols");
  orbit_object_utils::ObjectFileInfo object_file_info{main_module_->load_bias()};
  OUTCOME_TRY(auto&& symbols, orbit_symbols::SymbolHelper::LoadSymbolsFromFile(
                                  main_executable_debug_file, object_file_info));
  main_module_->AddSymbols(symbols);
  return outcome::success();
}

bool ClientGgp::InitCapture() {
  ErrorMessageOr<std::unique_ptr<ProcessData>> target_process_result =
      GetOrbitProcessByPid(options_.capture_pid);
  if (target_process_result.has_error()) {
    ORBIT_ERROR("Not able to set target process: %s", target_process_result.error().message());
    return false;
  }
  target_process_ = std::move(target_process_result.value());
  // Load the module and symbols
  ErrorMessageOr<void> result = LoadModuleAndSymbols();
  if (result.has_error()) {
    ORBIT_ERROR("Not possible to finish loading the module and symbols: %s",
                result.error().message());
    return false;
  }
  // Load selected functions
  LoadSelectedFunctions();
  return true;
}

void ClientGgp::LoadSelectedFunctions() {
  // Load selected functions if provided
  if (!options_.capture_functions.empty()) {
    ORBIT_LOG("Loading selected functions");
    selected_functions_ = GetSelectedFunctions();
    if (!selected_functions_.empty()) {
      ORBIT_LOG("List of selected functions to hook in the capture:");
      for (auto const& [address, selected_function] : selected_functions_) {
        ORBIT_LOG("%d %s", address, selected_function.pretty_name());
      }
    }
  } else {
    ORBIT_LOG("No functions provided; no functions hooked in the capture");
  }
}

void ClientGgp::InformUsedSelectedCaptureFunctions(
    const absl::flat_hash_set<std::string>& capture_functions_used) {
  if (capture_functions_used.size() != options_.capture_functions.size()) {
    for (const std::string& selected_function : options_.capture_functions) {
      if (!capture_functions_used.contains(selected_function)) {
        ORBIT_ERROR("Function matching %s not found; will not be hooked in the capture",
                    selected_function);
      }
    }
  } else {
    ORBIT_LOG("All functions provided had at least a match");
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
      selected_functions.insert_or_assign(function_id++, *func);
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
