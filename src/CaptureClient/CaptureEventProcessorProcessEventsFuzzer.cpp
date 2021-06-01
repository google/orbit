// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <libfuzzer/libfuzzer_macro.h>
#include <stdint.h>

#include <string>

#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureClient/CaptureListener.h"
#include "ClientData/ProcessData.h"
#include "ClientData/TracepointCustom.h"
#include "ClientData/UserDefinedCaptureData.h"
#include "OrbitBase/Result.h"
#include "absl/flags/flag.h"
#include "capture.pb.h"
#include "capture_data.pb.h"
#include "services.pb.h"
#include "tracepoint.pb.h"

ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");

namespace orbit_capture_client {

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::CaptureResponse;

namespace {
class MyCaptureListener : public CaptureListener {
 private:
  void OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& /*capture_started*/,
                        absl::flat_hash_set<uint64_t> /*frame_track_function_ids*/) override {}
  void OnCaptureFinished(const orbit_grpc_protos::CaptureFinished& /*capture_finished*/) override {}
  void OnTimer(const TimerInfo& /*timer_info*/) override {}
  void OnSystemMemoryUsage(
      const orbit_grpc_protos::SystemMemoryUsage& /*system_memory_usage*/) override {}
  void OnKeyAndString(uint64_t /*key*/, std::string /*str*/) override {}
  void OnUniqueCallstack(uint64_t /*callstack_id*/, CallstackInfo /*callstack*/) override {}
  void OnCallstackEvent(CallstackEvent /*callstack_event*/) override {}
  void OnThreadName(int32_t /*thread_id*/, std::string /*thread_name*/) override {}
  void OnThreadStateSlice(
      orbit_client_protos::ThreadStateSliceInfo /*thread_state_slice*/) override {}
  void OnAddressInfo(LinuxAddressInfo /*address_info*/) override {}
  void OnUniqueTracepointInfo(uint64_t /*key*/,
                              orbit_grpc_protos::TracepointInfo /*tracepoint_info*/) override {}
  void OnTracepointEvent(
      orbit_client_protos::TracepointEventInfo /*tracepoint_event_info*/) override {}
  void OnModuleUpdate(uint64_t /*timestamp_ns*/,
                      orbit_grpc_protos::ModuleInfo /*module_info*/) override {}
  void OnModulesSnapshot(uint64_t /*timestamp_ns*/,
                         std::vector<orbit_grpc_protos::ModuleInfo> /*module_infos*/) override {}
};
}  // namespace

DEFINE_PROTO_FUZZER(const CaptureResponse& response) {
  MyCaptureListener listener;
  auto processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});
  for (const auto& event : response.capture_events()) {
    processor->ProcessEvent(event);
  }
}

}  // namespace orbit_capture_client
