// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <gmock/gmock.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <libfuzzer/libfuzzer_macro.h>

#include <atomic>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <utility>

#include "CaptureClient/CaptureListener.h"
#include "ClientData/ModuleManager.h"
#include "ClientModel/CaptureDeserializer.h"
#include "ClientModel/CaptureSerializer.h"

namespace orbit_client_model {

namespace {
class MockCaptureListener : public orbit_capture_client::CaptureListener {
 public:
  void OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& /*capture_started*/,
                        std::optional<std::filesystem::path> /*file_path*/,
                        absl::flat_hash_set<uint64_t> /*frame_track_function_ids*/) override {}
  void OnCaptureFinished(const orbit_grpc_protos::CaptureFinished& /*capture_finished*/) override {}
  void OnTimer(const orbit_client_protos::TimerInfo& /*timer_info*/) override {}
  void OnSystemMemoryUsage(
      const orbit_grpc_protos::SystemMemoryUsage& /*system_memory_usage*/) override {}
  void OnKeyAndString(uint64_t /*key*/, std::string /*str*/) override {}
  void OnUniqueCallstack(uint64_t /*callstack_id*/,
                         orbit_client_protos::CallstackInfo /*callstack*/) override {}
  void OnCallstackEvent(orbit_client_protos::CallstackEvent /*callstack_event*/) override {}
  void OnThreadName(int32_t /*thread_id*/, std::string /*thread_name*/) override {}
  void OnThreadStateSlice(
      orbit_client_protos::ThreadStateSliceInfo /*thread_state_slice*/) override {}
  void OnAddressInfo(orbit_client_protos::LinuxAddressInfo /*address_info*/) override {}
  void OnUniqueTracepointInfo(uint64_t /*key*/,
                              orbit_grpc_protos::TracepointInfo /*tracepoint_info*/) override {}
  void OnTracepointEvent(
      orbit_client_protos::TracepointEventInfo /*tracepoint_event_info*/) override {}
  void OnModuleUpdate(uint64_t /*timestamp_ns*/,
                      orbit_grpc_protos::ModuleInfo /*module_info*/) override {}
  void OnModulesSnapshot(uint64_t /*timestamp_ns*/,
                         std::vector<orbit_grpc_protos::ModuleInfo> /*module_infos*/) override {}
};

void WriteMessage(const google::protobuf::Message* message,
                  google::protobuf::io::CodedOutputStream* output) {
  uint32_t message_size = message->ByteSizeLong();
  output->WriteLittleEndian32(message_size);
  message->SerializeToCodedStream(output);
}

}  // namespace

DEFINE_PROTO_FUZZER(const orbit_client_protos::CaptureDeserializerFuzzerInfo& info) {
  std::string buffer{};
  {
    google::protobuf::io::StringOutputStream stream{&buffer};
    google::protobuf::io::CodedOutputStream coded_stream{&stream};
    orbit_client_protos::CaptureHeader capture_header{};
    capture_header.set_version("1.59");

    WriteMessage(&capture_header, &coded_stream);
    WriteMessage(&info.capture_info(), &coded_stream);
    for (const auto& timer : info.timers()) {
      WriteMessage(&timer, &coded_stream);
    }
  }

  // NOLINTNEXTLINE
  google::protobuf::io::ArrayInputStream input_stream{buffer.data(),
                                                      static_cast<int>(buffer.size())};
  google::protobuf::io::CodedInputStream coded_input_stream{&input_stream};
  std::atomic<bool> cancellation_requested = false;

  MockCaptureListener capture_listener{};
  orbit_client_data::ModuleManager module_manager{};
  (void)capture_deserializer::Load(&coded_input_stream, info.filename(), &capture_listener,
                                   &module_manager, &cancellation_requested);
}

}  // namespace orbit_client_model
