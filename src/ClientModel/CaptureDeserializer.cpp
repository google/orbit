// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientModel/CaptureDeserializer.h"

#include <absl/container/flat_hash_map.h>
#include <absl/meta/type_traits.h>
#include <google/protobuf/stubs/port.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "CaptureClient/CaptureClient.h"
#include "CaptureClient/CaptureListener.h"
#include "ClientData/Callstack.h"
#include "ClientData/FunctionUtils.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "ClientData/TracepointCustom.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitBase/Result.h"
#include "absl/strings/str_format.h"
#include "capture_data.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"
#include "module.pb.h"
#include "process.pb.h"
#include "tracepoint.pb.h"

using orbit_capture_client::CaptureListener;

using orbit_client_data::CallStack;
using orbit_client_data::ModuleData;
using orbit_client_data::ModuleManager;
using orbit_client_data::ProcessData;
using orbit_client_data::TracepointInfoSet;

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;
using orbit_client_protos::CaptureHeader;
using orbit_client_protos::CaptureInfo;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::ProcessInfo;

namespace orbit_client_model::capture_deserializer {

ErrorMessageOr<CaptureListener::CaptureOutcome> Load(const std::filesystem::path& file_name,
                                                     CaptureListener* capture_listener,
                                                     ModuleManager* module_manager,
                                                     std::atomic<bool>* cancellation_requested) {
  SCOPED_TIMED_LOG("Loading capture from \"%s\"", file_name.string());

  auto fd_or_error = orbit_base::OpenFileForReading(file_name);
  if (fd_or_error.has_error()) {
    ERROR("%s", fd_or_error.error().message());
    return fd_or_error.error();
  }

  google::protobuf::io::FileInputStream input_stream(fd_or_error.value().get());
  google::protobuf::io::CodedInputStream coded_input(&input_stream);

  return Load(&coded_input, file_name, capture_listener, module_manager, cancellation_requested);
}

ErrorMessageOr<CaptureListener::CaptureOutcome> Load(
    google::protobuf::io::CodedInputStream* input_stream, const std::filesystem::path& file_name,
    CaptureListener* capture_listener, ModuleManager* module_manager,
    std::atomic<bool>* cancellation_requested) {
  std::string error_message = absl::StrFormat(
      "Error parsing the capture from \"%s\".\nNote: If the capture "
      "was taken with a previous Orbit version, it could be incompatible. "
      "Please check release notes for more information.",
      file_name.string());

  CaptureHeader header;
  if (!internal::ReadMessage(&header, input_stream) || header.version().empty()) {
    ERROR("%s", error_message);
    return ErrorMessage{std::move(error_message)};
  }
  if (header.version() != internal::kRequiredCaptureVersion) {
    std::string incompatible_version_error_message = absl::StrFormat(
        "The format of capture \"%s\" is no longer supported but could be opened with "
        "Orbit version %s.",
        file_name.string(), header.version());
    ERROR("%s", incompatible_version_error_message);
    return ErrorMessage{std::move(incompatible_version_error_message)};
  }

  CaptureInfo capture_info;
  if (!internal::ReadMessage(&capture_info, input_stream)) {
    ERROR("%s", error_message);
    return ErrorMessage{std::move(error_message)};
  }

  return internal::LoadCaptureInfo(capture_info, capture_listener, module_manager, input_stream,
                                   cancellation_requested);
}

namespace internal {

bool ReadMessage(google::protobuf::Message* message,
                 google::protobuf::io::CodedInputStream* input) {
  uint32_t message_size;
  if (!input->ReadLittleEndian32(&message_size)) {
    return false;
  }

  std::unique_ptr<char[]> buffer = make_unique_for_overwrite<char[]>(message_size);
  if (!input->ReadRaw(buffer.get(), message_size)) {
    return false;
  }
  message->ParseFromArray(buffer.get(), message_size);

  return true;
}

ErrorMessageOr<CaptureListener::CaptureOutcome> LoadCaptureInfo(
    const CaptureInfo& capture_info, CaptureListener* capture_listener,
    ModuleManager* module_manager, google::protobuf::io::CodedInputStream* coded_input,
    std::atomic<bool>* cancellation_requested) {
  CHECK(capture_listener != nullptr);

  ProcessInfo process_info;
  process_info.set_pid(capture_info.process().pid());
  process_info.set_name(capture_info.process().name());
  process_info.set_cpu_usage(capture_info.process().cpu_usage());
  process_info.set_full_path(capture_info.process().full_path());
  process_info.set_command_line(capture_info.process().command_line());
  process_info.set_is_64_bit(capture_info.process().is_64_bit());
  ProcessData process(process_info);

  if (*cancellation_requested) {
    return CaptureListener::CaptureOutcome::kCancelled;
  }

  std::vector<orbit_grpc_protos::ModuleInfo> modules;
  absl::flat_hash_map<std::string, orbit_grpc_protos::ModuleInfo> module_map;
  for (const auto& module : capture_info.modules()) {
    orbit_grpc_protos::ModuleInfo module_info;
    module_info.set_name(module.name());
    module_info.set_file_path(module.file_path());
    module_info.set_file_size(module.file_size());
    module_info.set_address_start(module.address_start());
    module_info.set_address_end(module.address_end());
    module_info.set_build_id(module.build_id());
    module_info.set_load_bias(module.load_bias());
    modules.emplace_back(std::move(module_info));
    module_map[module.file_path()] = modules.back();
  }
  process.UpdateModuleInfos(modules);

  CHECK(module_manager->AddOrUpdateModules(modules).empty());

  if (*cancellation_requested) {
    return CaptureListener::CaptureOutcome::kCancelled;
  }

  CaptureStarted capture_started;
  capture_started.set_process_id(capture_info.process().pid());
  capture_started.set_executable_path(process.full_path());
  capture_started.set_process_id(process.pid());

  CaptureOptions* capture_options = capture_started.mutable_capture_options();
  capture_options->set_pid(capture_info.process().pid());
  absl::flat_hash_map<uint64_t, orbit_grpc_protos::InstrumentedFunction> instrumented_functions;
  for (const auto& function : capture_info.instrumented_functions()) {
    orbit_grpc_protos::InstrumentedFunction instrumented_function;
    instrumented_function.set_function_id(function.first);
    instrumented_function.set_function_name(function.second.pretty_name());
    instrumented_function.set_file_path(function.second.module_path());
    ModuleData* module_data = module_manager->GetMutableModuleByPathAndBuildId(
        function.second.module_path(), function.second.module_build_id());

    // In the case module_data was not found by build id check if FunctionInfo build_id is empty,
    // in which case assume this is capture saved with Orbit v1.61 and below try to get module
    // build id from path and do another lookup.
    if (module_data == nullptr && function.second.module_build_id().empty()) {
      const std::string& module_path = function.second.module_path();
      CHECK(module_map.contains(module_path));
      const std::string& build_id = module_map.at(module_path).build_id();
      module_data = module_manager->GetMutableModuleByPathAndBuildId(module_path, build_id);
    }

    CHECK(module_data != nullptr);
    instrumented_function.set_file_build_id(module_data->build_id());
    instrumented_function.set_file_offset(
        orbit_client_data::function_utils::Offset(function.second, *module_data));
    instrumented_functions.insert_or_assign(function.first, instrumented_function);

    module_data->AddFunctionInfoWithBuildId(function.second, module_data->build_id());
    *capture_options->add_instrumented_functions() = std::move(instrumented_function);
  }

  TracepointInfoSet selected_tracepoints;
  for (const orbit_client_protos::TracepointInfo& tracepoint_info :
       capture_info.tracepoint_infos()) {
    orbit_grpc_protos::TracepointInfo tracepoint_info_translated;
    tracepoint_info_translated.set_category(tracepoint_info.category());
    tracepoint_info_translated.set_name(tracepoint_info.name());
    selected_tracepoints.emplace(tracepoint_info_translated);
  }

  if (*cancellation_requested) {
    return CaptureListener::CaptureOutcome::kCancelled;
  }

  absl::flat_hash_set<uint64_t> frame_track_function_ids;
  for (uint64_t function_id :
       capture_info.user_defined_capture_info().frame_tracks_info().frame_track_function_ids()) {
    frame_track_function_ids.insert(function_id);
  }

  capture_listener->OnCaptureStarted(capture_started, frame_track_function_ids);

  for (const auto& address_info : capture_info.address_infos()) {
    if (*cancellation_requested) {
      return CaptureListener::CaptureOutcome::kCancelled;
    }
    capture_listener->OnAddressInfo(address_info);
  }

  for (const auto& thread_id_and_name : capture_info.thread_names()) {
    if (*cancellation_requested) {
      return CaptureListener::CaptureOutcome::kCancelled;
    }
    capture_listener->OnThreadName(thread_id_and_name.first, thread_id_and_name.second);
  }

  for (const orbit_client_protos::ThreadStateSliceInfo& thread_state_slice :
       capture_info.thread_state_slices()) {
    if (*cancellation_requested) {
      return CaptureListener::CaptureOutcome::kCancelled;
    }
    capture_listener->OnThreadStateSlice(thread_state_slice);
  }

  for (const auto& id_and_callstack_info : capture_info.callstacks()) {
    uint64_t callstack_id = id_and_callstack_info.first;
    const CallstackInfo& callstack = id_and_callstack_info.second;
    CallStack unique_callstack({callstack.frames().begin(), callstack.frames().end()});
    if (*cancellation_requested) {
      return CaptureListener::CaptureOutcome::kCancelled;
    }
    capture_listener->OnUniqueCallStack(callstack_id, std::move(unique_callstack));
  }
  for (CallstackEvent callstack_event : capture_info.callstack_events()) {
    if (*cancellation_requested) {
      return CaptureListener::CaptureOutcome::kCancelled;
    }
    capture_listener->OnCallstackEvent(std::move(callstack_event));
  }

  for (const orbit_client_protos::TracepointInfo& tracepoint_info :
       capture_info.tracepoint_infos()) {
    if (*cancellation_requested) {
      return CaptureListener::CaptureOutcome::kCancelled;
    }
    orbit_grpc_protos::TracepointInfo tracepoint_info_translated;
    tracepoint_info_translated.set_category(tracepoint_info.category());
    tracepoint_info_translated.set_name(tracepoint_info.name());
    capture_listener->OnUniqueTracepointInfo(tracepoint_info.tracepoint_info_key(),
                                             std::move(tracepoint_info_translated));
  }

  for (orbit_client_protos::TracepointEventInfo tracepoint_event_info :
       capture_info.tracepoint_event_infos()) {
    if (*cancellation_requested) {
      return CaptureListener::CaptureOutcome::kCancelled;
    }
    capture_listener->OnTracepointEvent(std::move(tracepoint_event_info));
  }

  for (const auto& key_to_string : capture_info.key_to_string()) {
    if (*cancellation_requested) {
      return CaptureListener::CaptureOutcome::kCancelled;
    }
    capture_listener->OnKeyAndString(key_to_string.first, key_to_string.second);
  }

  // Timers
  TimerInfo timer_info;
  while (internal::ReadMessage(&timer_info, coded_input)) {
    if (*cancellation_requested) {
      return CaptureListener::CaptureOutcome::kCancelled;
    }
    capture_listener->OnTimer(timer_info);
  }

  return CaptureListener::CaptureOutcome::kComplete;
}

}  // namespace internal

}  // namespace orbit_client_model::capture_deserializer
