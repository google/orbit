// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureSerializer.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>

#include <fstream>
#include <memory>

#include "App.h"
#include "Callstack.h"
#include "EventTracer.h"
#include "FunctionUtils.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitProcess.h"
#include "SamplingProfiler.h"
#include "TimeGraph.h"
#include "absl/strings/str_format.h"
#include "capture_data.pb.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;
using orbit_client_protos::CaptureInfo;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::FunctionStats;
using orbit_client_protos::TimerInfo;

void CaptureSerializer::WriteMessage(const google::protobuf::Message* message,
                                     google::protobuf::io::CodedOutputStream* output) {
  uint32_t message_size = message->ByteSizeLong();
  output->WriteLittleEndian32(message_size);
  message->SerializeToCodedStream(output);
}

CaptureInfo CaptureSerializer::GenerateCaptureInfo(
    const CaptureData& capture_data,
    const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map) {
  CaptureInfo capture_info;
  for (const auto& pair : capture_data.selected_functions()) {
    capture_info.add_selected_functions()->CopyFrom(pair.second);
  }

  capture_info.set_process_id(capture_data.process_id());
  capture_info.set_process_name(capture_data.process_name());

  capture_info.mutable_thread_names()->insert(capture_data.thread_names().begin(),
                                              capture_data.thread_names().end());

  capture_info.mutable_address_infos()->Reserve(capture_data.address_infos().size());
  for (const auto& address_info : capture_data.address_infos()) {
    orbit_client_protos::LinuxAddressInfo* added_address_info = capture_info.add_address_infos();
    added_address_info->CopyFrom(address_info.second);
    // Fix names in address infos (some might only be in process):
    added_address_info->set_function_name(
        capture_data.GetFunctionNameByAddress(added_address_info->absolute_address()));
  }

  const absl::flat_hash_map<uint64_t, FunctionStats>& functions_stats =
      capture_data.functions_stats();
  capture_info.mutable_function_stats()->insert(functions_stats.begin(), functions_stats.end());

  // TODO: this is not really synchronized, since GetCallstacks processing below
  // is not under the same mutex lock we could end up having list of callstacks
  // inconsistent with unique_callstacks. Revisit sampling profiler data
  // thread-safety.
  capture_data.GetCallstackData()->ForEachUniqueCallstack(
      [&capture_info](const CallStack& call_stack) {
        CallstackInfo* callstack = capture_info.add_callstacks();
        *callstack->mutable_data() = {call_stack.GetFrames().begin(), call_stack.GetFrames().end()};
      });

  const auto& callstacks = capture_data.GetCallstackData()->callstack_events();
  capture_info.mutable_callstack_events()->Reserve(callstacks.size());
  for (const auto& callstack : callstacks) {
    capture_info.add_callstack_events()->CopyFrom(callstack);
  }

  capture_info.mutable_key_to_string()->insert(key_to_string_map.begin(), key_to_string_map.end());

  return capture_info;
}

ErrorMessageOr<void> CaptureSerializer::Load(const std::string& filename) {
  SCOPE_TIMER_LOG(absl::StrFormat("Loading capture from \"%s\"", filename));

  // Binary
  std::ifstream file(filename, std::ios::binary);
  if (file.fail()) {
    ERROR("Loading capture from \"%s\": %s", filename, "file.fail()");
    return ErrorMessage("Error opening the file for reading");
  }

  return Load(file);
}

bool CaptureSerializer::ReadMessage(google::protobuf::Message* message,
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

static void FillEventBuffer(const CaptureData& capture_data) {
  GEventTracer.GetEventBuffer().Reset();
  for (const CallstackEvent& callstack_event :
       capture_data.GetCallstackData()->callstack_events()) {
    GEventTracer.GetEventBuffer().AddCallstackEvent(
        callstack_event.time(), callstack_event.callstack_hash(), callstack_event.thread_id());
  }
}

CaptureData CaptureSerializer::GenerateCaptureData(const CaptureInfo& capture_info) {
  // Clear the old capture
  GOrbitApp->ClearSelectedFunctions();
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions;
  absl::flat_hash_set<uint64_t> visible_functions;
  for (const auto& function : capture_info.selected_functions()) {
    uint64_t address = FunctionUtils::GetAbsoluteAddress(function);
    selected_functions[address] = function;
    visible_functions.insert(address);
  }
  GOrbitApp->SetVisibleFunctions(std::move(visible_functions));

  absl::flat_hash_map<uint64_t, FunctionStats> functions_stats{
      capture_info.function_stats().begin(), capture_info.function_stats().end()};
  CaptureData capture_data(capture_info.process_id(), capture_info.process_name(),
                           std::make_shared<Process>(), std::move(selected_functions),
                           std::move(functions_stats));

  absl::flat_hash_map<uint64_t, orbit_client_protos::LinuxAddressInfo> address_infos;
  address_infos.reserve(capture_info.address_infos_size());
  for (const auto& address_info : capture_info.address_infos()) {
    address_infos[address_info.absolute_address()] = address_info;
  }
  capture_data.set_address_infos(std::move(address_infos));
  absl::flat_hash_map<int32_t, std::string> thread_names{capture_info.thread_names().begin(),
                                                         capture_info.thread_names().end()};
  capture_data.set_thread_names(thread_names);

  for (const CallstackInfo& callstack : capture_info.callstacks()) {
    CallStack unique_callstack({callstack.data().begin(), callstack.data().end()});
    capture_data.AddUniqueCallStack(std::move(unique_callstack));
  }
  for (CallstackEvent callstack_event : capture_info.callstack_events()) {
    capture_data.AddCallstackEvent(std::move(callstack_event));
  }
  SamplingProfiler sampling_profiler(*capture_data.GetCallstackData(), capture_data);
  capture_data.set_sampling_profiler(sampling_profiler);

  time_graph_->Clear();
  StringManager* string_manager = time_graph_->GetStringManager();
  string_manager->Clear();
  for (const auto& entry : capture_info.key_to_string()) {
    string_manager->AddIfNotPresent(entry.first, entry.second);
  }

  FillEventBuffer(capture_data);

  return capture_data;
}

ErrorMessageOr<void> CaptureSerializer::Load(std::istream& stream) {
  google::protobuf::io::IstreamInputStream input_stream(&stream);
  google::protobuf::io::CodedInputStream coded_input(&input_stream);

  std::string error_message =
      "Error parsing the capture.\nNote: If the capture "
      "was taken with a previous Orbit version, it could be incompatible. "
      "Please check release notes for more information.";

  if (!ReadMessage(&header_, &coded_input) || header_.version().empty()) {
    ERROR("%s", error_message);
    return ErrorMessage(error_message);
  }
  if (header_.version() != kRequiredCaptureVersion) {
    std::string incompatible_version_error_message = absl::StrFormat(
        "This capture format is no longer supported but could be opened with "
        "Orbit version %s.",
        header_.version());
    ERROR("%s", incompatible_version_error_message);
    return ErrorMessage(incompatible_version_error_message);
  }

  CaptureInfo capture_info;
  if (!ReadMessage(&capture_info, &coded_input)) {
    ERROR("%s", error_message);
    return ErrorMessage(error_message);
  }
  CaptureData capture_data = GenerateCaptureData(capture_info);

  // Timers
  TimerInfo timer_info;
  while (ReadMessage(&timer_info, &coded_input)) {
    if (timer_info.function_address() > 0) {
      const FunctionInfo& func =
          capture_data.selected_functions().at(timer_info.function_address());
      time_graph_->ProcessTimer(timer_info, &func);
    } else {
      time_graph_->ProcessTimer(timer_info, nullptr);
    }
  }

  GOrbitApp->SetSamplingReport(capture_data.sampling_profiler(),
                               capture_data.GetCallstackData()->GetUniqueCallstacksCopy());
  GOrbitApp->SetTopDownView(capture_data);
  GOrbitApp->SetCaptureData(std::move(capture_data));
  GOrbitApp->FireRefreshCallbacks();
  return outcome::success();
}
