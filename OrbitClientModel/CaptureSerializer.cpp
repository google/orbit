// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientModel/CaptureSerializer.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>

#include <fstream>
#include <memory>

#include "Callstack.h"
#include "EventTracer.h"
#include "FunctionUtils.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitProcess.h"
#include "absl/strings/str_format.h"
#include "capture_data.pb.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;
using orbit_client_protos::CaptureInfo;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::FunctionStats;
using orbit_client_protos::TimerInfo;

void CaptureDeserializer::WriteMessage(const google::protobuf::Message* message,
                                       google::protobuf::io::CodedOutputStream* output) {
  uint32_t message_size = message->ByteSizeLong();
  output->WriteLittleEndian32(message_size);
  message->SerializeToCodedStream(output);
}

CaptureInfo CaptureDeserializer::GenerateCaptureInfo(
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

  // TODO: this is not really synchronized, since GetCallstackData processing below is not under the
  // same mutex lock we could end up having list of callstacks inconsistent with unique_callstacks.
  // Revisit sampling profiler data thread-safety.
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
