// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <libfuzzer/libfuzzer_macro.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureClient/CaptureListener.h"
#include "ClientData/ApiStringEvent.h"
#include "ClientData/ApiTrackValue.h"
#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CgroupAndProcessMemoryInfo.h"
#include "ClientData/LinuxAddressInfo.h"
#include "ClientData/PageFaultsInfo.h"
#include "ClientData/SystemMemoryInfo.h"
#include "ClientData/ThreadStateSliceInfo.h"
#include "ClientData/TracepointEventInfo.h"
#include "ClientData/TracepointInfo.h"
#include "ClientProtos/capture_data.pb.h"
#include "FuzzingUtils/ProtoFuzzer.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/services.pb.h"

namespace orbit_capture_client {

using orbit_client_data::CallstackEvent;
using orbit_client_data::CallstackInfo;
using orbit_client_data::LinuxAddressInfo;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::CaptureResponse;

namespace {
class MyCaptureListener : public CaptureListener {
 private:
  void OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& /*capture_started*/,
                        std::optional<std::filesystem::path> /*file_path*/,
                        absl::flat_hash_set<uint64_t> /*frame_track_function_ids*/) override {}
  void OnCaptureFinished(const orbit_grpc_protos::CaptureFinished& /*capture_finished*/) override {}
  void OnTimer(const TimerInfo& /*timer_info*/) override {}
  void OnCgroupAndProcessMemoryInfo(const orbit_client_data::CgroupAndProcessMemoryInfo&
                                    /*cgroup_and_process_memory_info*/) override {}
  void OnPageFaultsInfo(const orbit_client_data::PageFaultsInfo& /*page_faults_info*/) override {}
  void OnSystemMemoryInfo(
      const orbit_client_data::SystemMemoryInfo& /*system_memory_info*/) override {}
  void OnKeyAndString(uint64_t /*key*/, std::string /*str*/) override {}
  void OnUniqueCallstack(uint64_t /*callstack_id*/, CallstackInfo /*callstack*/) override {}
  void OnCallstackEvent(CallstackEvent /*callstack_event*/) override {}
  void OnThreadName(uint32_t /*thread_id*/, std::string /*thread_name*/) override {}
  void OnThreadStateSlice(orbit_client_data::ThreadStateSliceInfo /*thread_state_slice*/) override {
  }
  void OnAddressInfo(LinuxAddressInfo /*address_info*/) override {}
  void OnUniqueTracepointInfo(uint64_t /*tracepoint_id*/,
                              orbit_client_data::TracepointInfo /*tracepoint_info*/) override {}
  void OnTracepointEvent(
      orbit_client_data::TracepointEventInfo /*tracepoint_event_info*/) override {}
  void OnModuleUpdate(uint64_t /*timestamp_ns*/,
                      orbit_grpc_protos::ModuleInfo /*module_info*/) override {}
  void OnModulesSnapshot(uint64_t /*timestamp_ns*/,
                         std::vector<orbit_grpc_protos::ModuleInfo> /*module_infos*/) override {}
  void OnPresentEvent(const orbit_grpc_protos::PresentEvent& /*present_event*/) override {}
  void OnApiStringEvent(const orbit_client_data::ApiStringEvent& /*api_string_event*/) override {}
  void OnApiTrackValue(const orbit_client_data::ApiTrackValue& /*api_track_value*/) override {}
  void OnWarningEvent(orbit_grpc_protos::WarningEvent /*warning_event*/) override {}
  void OnClockResolutionEvent(
      orbit_grpc_protos::ClockResolutionEvent /*clock_resolution_event*/) override {}
  void OnErrorsWithPerfEventOpenEvent(
      orbit_grpc_protos::ErrorsWithPerfEventOpenEvent /*errors_with_perf_event_open_event*/)
      override {}
  void OnWarningInstrumentingWithUprobesEvent(
      orbit_grpc_protos::WarningInstrumentingWithUprobesEvent
      /*warning_instrumenting_with_uprobes_event*/) override {}
  void OnErrorEnablingOrbitApiEvent(
      orbit_grpc_protos::ErrorEnablingOrbitApiEvent /*error_enabling_orbit_api_event*/) override {}
  void OnErrorEnablingUserSpaceInstrumentationEvent(
      orbit_grpc_protos::ErrorEnablingUserSpaceInstrumentationEvent /*error_event*/) override {}
  void OnWarningInstrumentingWithUserSpaceInstrumentationEvent(
      orbit_grpc_protos::WarningInstrumentingWithUserSpaceInstrumentationEvent /*warning_event*/)
      override {}
  void OnLostPerfRecordsEvent(
      orbit_grpc_protos::LostPerfRecordsEvent /*lost_perf_records_event*/) override {}
  void OnOutOfOrderEventsDiscardedEvent(orbit_grpc_protos::OutOfOrderEventsDiscardedEvent
                                        /*out_of_order_events_discarded_event*/) override {}
};
}  // namespace

ORBIT_DEFINE_PROTO_FUZZER(const CaptureResponse& response) {
  MyCaptureListener listener;
  auto processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});
  for (const auto& event : response.capture_events()) {
    processor->ProcessEvent(event);
  }
}

}  // namespace orbit_capture_client
