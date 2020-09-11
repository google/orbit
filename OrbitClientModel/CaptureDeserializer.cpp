// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientModel/CaptureDeserializer.h"

#include <fstream>
#include <memory>

#include "Callstack.h"
#include "FunctionUtils.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "absl/strings/str_format.h"
#include "capture_data.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;
using orbit_client_protos::CaptureHeader;
using orbit_client_protos::CaptureInfo;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::TimerInfo;

namespace capture_deserializer {

ErrorMessageOr<void> Load(const std::string& filename, CaptureListener* capture_listener,
                          std::atomic<bool>* cancellation_requested) {
  SCOPE_TIMER_LOG(absl::StrFormat("Loading capture from \"%s\"", filename));

  // Binary
  std::ifstream file(filename, std::ios::binary);
  if (file.fail()) {
    ERROR("Loading capture from \"%s\": %s", filename, "file.fail()");
    return ErrorMessage("Error opening the file for reading");
  }

  return Load(file, capture_listener, cancellation_requested);
}

ErrorMessageOr<void> Load(std::istream& stream, CaptureListener* capture_listener,
                          std::atomic<bool>* cancellation_requested) {
  google::protobuf::io::IstreamInputStream input_stream(&stream);
  google::protobuf::io::CodedInputStream coded_input(&input_stream);

  std::string error_message =
      "Error parsing the capture.\nNote: If the capture "
      "was taken with a previous Orbit version, it could be incompatible. "
      "Please check release notes for more information.";

  CaptureHeader header;
  if (!internal::ReadMessage(&header, &coded_input) || header.version().empty()) {
    ERROR("%s", error_message);
    return ErrorMessage(error_message);
  }
  if (header.version() != internal::kRequiredCaptureVersion) {
    std::string incompatible_version_error_message = absl::StrFormat(
        "This capture format is no longer supported but could be opened with "
        "Orbit version %s.",
        header.version());
    ERROR("%s", incompatible_version_error_message);
    return ErrorMessage(incompatible_version_error_message);
  }

  CaptureInfo capture_info;
  if (!internal::ReadMessage(&capture_info, &coded_input)) {
    ERROR("%s", error_message);
    return ErrorMessage(error_message);
  }

  internal::LoadCaptureInfo(capture_info, capture_listener, &coded_input, cancellation_requested);

  return outcome::success();
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

void LoadCaptureInfo(const CaptureInfo& capture_info, CaptureListener* capture_listener,
                     google::protobuf::io::CodedInputStream* coded_input,
                     std::atomic<bool>* cancellation_requested) {
  CHECK(capture_listener != nullptr);

  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions;
  for (const auto& function : capture_info.selected_functions()) {
    uint64_t address = FunctionUtils::GetAbsoluteAddress(function);
    selected_functions[address] = function;
  }
  TracepointInfoSet selected_tracepoints;

  if (*cancellation_requested) {
    return;
  }
  capture_listener->OnCaptureStarted(capture_info.process_id(), capture_info.process_name(),
                                     std::make_shared<Process>(), std::move(selected_functions),
                                     std::move(selected_tracepoints));

  for (const auto& address_info : capture_info.address_infos()) {
    if (*cancellation_requested) {
      return;
    }
    capture_listener->OnAddressInfo(address_info);
  }

  for (const auto& thread_id_and_name : capture_info.thread_names()) {
    if (*cancellation_requested) {
      return;
    }
    capture_listener->OnThreadName(thread_id_and_name.first, thread_id_and_name.second);
  }

  for (const CallstackInfo& callstack : capture_info.callstacks()) {
    CallStack unique_callstack({callstack.data().begin(), callstack.data().end()});
    if (*cancellation_requested) {
      return;
    }
    capture_listener->OnUniqueCallStack(std::move(unique_callstack));
  }
  for (CallstackEvent callstack_event : capture_info.callstack_events()) {
    if (*cancellation_requested) {
      return;
    }
    capture_listener->OnCallstackEvent(std::move(callstack_event));
  }

  for (const auto& key_to_string : capture_info.key_to_string()) {
    if (*cancellation_requested) {
      return;
    }
    capture_listener->OnKeyAndString(key_to_string.first, key_to_string.second);
  }

  // Timers
  TimerInfo timer_info;
  while (internal::ReadMessage(&timer_info, coded_input)) {
    if (*cancellation_requested) {
      return;
    }
    capture_listener->OnTimer(timer_info);
  }

  if (*cancellation_requested) {
    return;
  }
  capture_listener->OnCaptureComplete();
}

}  // namespace internal

}  // namespace capture_deserializer
