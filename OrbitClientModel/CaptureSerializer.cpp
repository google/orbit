// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientModel/CaptureSerializer.h"

#include <OrbitClientData/FunctionUtils.h>

#include <memory>

#include "CoreUtils.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitClientData/Callstack.h"
#include "capture_data.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/message.h"

using orbit_client_protos::CallstackInfo;
using orbit_client_protos::CaptureInfo;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::FunctionStats;
using orbit_client_protos::ModuleInfo;
using orbit_client_protos::ProcessInfo;

namespace {
inline constexpr std::string_view kFileOrbitExtension = ".orbit";
}

namespace capture_serializer {

void WriteMessage(const google::protobuf::Message* message,
                  google::protobuf::io::CodedOutputStream* output) {
  uint32_t message_size = message->ByteSizeLong();
  output->WriteLittleEndian32(message_size);
  message->SerializeToCodedStream(output);
}

std::string GetCaptureFileName(const CaptureData& capture_data) {
  time_t timestamp = std::chrono::system_clock::to_time_t(capture_data.capture_start_time());
  std::string result =
      absl::StrCat(std::filesystem::path(capture_data.process_name()).stem().string(), "_",
                   orbit_core::FormatTime(timestamp));
  IncludeOrbitExtensionInFile(result);
  return result;
}

void IncludeOrbitExtensionInFile(std::string& file_name) {
  const std::string extension = std::filesystem::path(file_name).extension().string();
  if (extension != kFileOrbitExtension) {
    file_name.append(kFileOrbitExtension);
  }
}

namespace internal {

CaptureInfo GenerateCaptureInfo(
    const CaptureData& capture_data,
    const absl::flat_hash_map<uint64_t, std::string>& key_to_string_map) {
  CaptureInfo capture_info;
  for (const auto& pair : capture_data.selected_functions()) {
    capture_info.add_selected_functions()->CopyFrom(pair.second);
  }

  ProcessInfo* process = capture_info.mutable_process();
  process->set_pid(capture_data.process()->pid());
  process->set_name(capture_data.process()->name());
  process->set_cpu_usage(capture_data.process()->cpu_usage());
  process->set_full_path(capture_data.process()->full_path());
  process->set_command_line(capture_data.process()->command_line());
  process->set_is_64_bit(capture_data.process()->is_64_bit());

  for (const auto& [module_path, memory_space] : capture_data.process()->GetMemoryMap()) {
    const ModuleData* module = capture_data.GetModuleByPath(module_path);
    CHECK(module != nullptr);
    ModuleInfo* module_info = capture_info.add_modules();
    module_info->set_name(module->name());
    module_info->set_file_path(module->file_path());
    module_info->set_file_size(module->file_size());
    module_info->set_address_start(memory_space.start);
    module_info->set_address_end(memory_space.end);
    module_info->set_build_id(module->build_id());
    module_info->set_load_bias(module->load_bias());
  }

  capture_info.mutable_thread_names()->insert(capture_data.thread_names().begin(),
                                              capture_data.thread_names().end());

  for (const auto& tid_and_thread_state_slices : capture_data.thread_state_slices()) {
    // Note that thread state slices are saved in their original order only among the same thread,
    // but all slices related to the same thread are saved sequentially. This might not be desired
    // if the capture is opened in a streaming fashion.
    for (const auto& thread_state_slice : tid_and_thread_state_slices.second) {
      orbit_client_protos::ThreadStateSliceInfo* added_thread_state_slice =
          capture_info.add_thread_state_slices();
      added_thread_state_slice->CopyFrom(thread_state_slice);
    }
  }

  capture_info.mutable_address_infos()->Reserve(capture_data.address_infos().size());
  for (const auto& address_info : capture_data.address_infos()) {
    orbit_client_protos::LinuxAddressInfo* added_address_info = capture_info.add_address_infos();
    added_address_info->CopyFrom(address_info.second);
    const uint64_t absolute_address = added_address_info->absolute_address();
    const orbit_client_protos::FunctionInfo* function =
        capture_data.FindFunctionByAddress(absolute_address, false);
    if (function == nullptr) {
      continue;
    }
    // Fix names/offset/module in address infos (some might only be in process):
    added_address_info->set_function_name(function_utils::GetDisplayName(*function));
    uint64_t absolute_function_address = capture_data.GetAbsoluteAddress(*function);
    const uint64_t offset = absolute_address - absolute_function_address;
    added_address_info->set_offset_in_function(offset);
    added_address_info->set_module_path(function->loaded_module_path());
  }

  const FunctionInfoMap<FunctionStats>& functions_stats = capture_data.functions_stats();
  for (const auto& [function, stats] : functions_stats) {
    uint64_t absolute_address = capture_data.GetAbsoluteAddress(function);
    capture_info.mutable_function_stats()->operator[](absolute_address) = stats;
  }

  // TODO: this is not really synchronized, since GetCallstackData processing below is not under the
  // same mutex lock we could end up having list of callstacks inconsistent with unique_callstacks.
  // Revisit sampling profiler data thread-safety.
  capture_data.GetCallstackData()->ForEachUniqueCallstack(
      [&capture_info](const CallStack& call_stack) {
        CallstackInfo* callstack = capture_info.add_callstacks();
        *callstack->mutable_data() = {call_stack.GetFrames().begin(), call_stack.GetFrames().end()};
      });

  capture_info.mutable_callstack_events()->Reserve(
      capture_data.GetCallstackData()->GetCallstackEventsCount());
  capture_data.GetCallstackData()->ForEachCallstackEvent(
      [&capture_info](const orbit_client_protos::CallstackEvent& event) {
        capture_info.add_callstack_events()->CopyFrom(event);
      });

  capture_data.GetTracepointData()->ForEachUniqueTracepointInfo(
      [&capture_info](const orbit_client_protos::TracepointInfo& tracepoint_info) {
        orbit_client_protos::TracepointInfo* new_tracepoint_info =
            capture_info.add_tracepoint_infos();
        *new_tracepoint_info->mutable_category() = tracepoint_info.category();
        *new_tracepoint_info->mutable_name() = tracepoint_info.name();
        new_tracepoint_info->set_tracepoint_info_key(tracepoint_info.tracepoint_info_key());
      });

  capture_data.GetTracepointData()->ForEachTracepointEvent(
      [&capture_info](const orbit_client_protos::TracepointEventInfo& tracepoint_event_info) {
        capture_info.add_tracepoint_event_infos()->CopyFrom(tracepoint_event_info);
      });

  capture_info.mutable_key_to_string()->insert(key_to_string_map.begin(), key_to_string_map.end());

  for (const auto& function : capture_data.user_defined_capture_data().frame_track_functions()) {
    capture_info.mutable_user_defined_capture_info()
        ->mutable_frame_tracks_info()
        ->add_frame_track_functions()
        ->CopyFrom(function);
  }

  return capture_info;
}

}  // namespace internal

}  // namespace capture_serializer
