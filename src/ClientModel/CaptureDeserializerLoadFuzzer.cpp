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

using orbit_client_data::CallStack;

namespace orbit_client_model {

namespace {
class MockCaptureListener : public orbit_capture_client::CaptureListener {
 public:
  void OnCaptureStarted(const orbit_grpc_protos::CaptureStarted&,
                        absl::flat_hash_set<uint64_t>) override {}
  void OnCaptureFinished(const orbit_grpc_protos::CaptureFinished&) override {}
  void OnTimer(const orbit_client_protos::TimerInfo&) override {}
  void OnSystemMemoryUsage(const orbit_grpc_protos::SystemMemoryUsage&) override {}
  void OnKeyAndString(uint64_t, std::string) override {}
  void OnUniqueCallStack(uint64_t, CallStack) override {}
  void OnCallstackEvent(orbit_client_protos::CallstackEvent) override {}
  void OnThreadName(int32_t, std::string) override {}
  void OnThreadStateSlice(orbit_client_protos::ThreadStateSliceInfo) override {}
  void OnAddressInfo(orbit_client_protos::LinuxAddressInfo) override {}
  void OnUniqueTracepointInfo(uint64_t, orbit_grpc_protos::TracepointInfo) override {}
  void OnTracepointEvent(orbit_client_protos::TracepointEventInfo) override {}
  void OnModuleUpdate(uint64_t, orbit_grpc_protos::ModuleInfo) override {}
  void OnModulesSnapshot(uint64_t, std::vector<orbit_grpc_protos::ModuleInfo>) override {}
};

}  // namespace

DEFINE_PROTO_FUZZER(const orbit_client_protos::CaptureDeserializerFuzzerInfo& info) {
  std::string buffer{};
  {
    google::protobuf::io::StringOutputStream stream{&buffer};
    google::protobuf::io::CodedOutputStream coded_stream{&stream};
    orbit_client_protos::CaptureHeader capture_header{};
    capture_header.set_version("1.59");

    capture_serializer::WriteMessage(&capture_header, &coded_stream);
    capture_serializer::WriteMessage(&info.capture_info(), &coded_stream);
    for (const auto& timer : info.timers()) {
      capture_serializer::WriteMessage(&timer, &coded_stream);
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
