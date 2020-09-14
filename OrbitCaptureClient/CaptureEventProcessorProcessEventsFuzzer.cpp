// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <libfuzzer/libfuzzer_macro.h>

#include "OrbitBase/Result.h"
#include "OrbitCaptureClient/CaptureEventProcessor.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "TracepointCustom.h"
#include "absl/flags/flag.h"
#include "services.pb.h"

ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");

using orbit_grpc_protos::CaptureResponse;

namespace {
using orbit_client_protos::CallstackEvent;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::TimerInfo;

class MyCaptureListener : public CaptureListener {
 private:
  void OnCaptureStarted(
      int32_t /*process_id*/, std::string /*process_name*/, std::shared_ptr<Process> /*process*/,
      absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> /*selected_functions*/,
      TracepointInfoSet /*selected_tracepoints*/) override {}
  void OnCaptureComplete() override {}
  void OnCaptureCancelled() override {}
  void OnCaptureFailed(ErrorMessage /*error_message*/) override {}
  void OnTimer(const TimerInfo&) override {}
  void OnKeyAndString(uint64_t, std::string) override {}
  void OnUniqueCallStack(CallStack) override {}
  void OnCallstackEvent(CallstackEvent) override {}
  void OnThreadName(int32_t, std::string) override {}
  void OnAddressInfo(LinuxAddressInfo) override {}
  void OnUniqueTracepointInfo(uint64_t, orbit_grpc_protos::TracepointInfo) override {}
  void OnTracepointEvent(orbit_client_protos::TracepointEventInfo) override {}
};
}  // namespace

DEFINE_PROTO_FUZZER(const CaptureResponse& response) {
  MyCaptureListener listener;
  CaptureEventProcessor processor{&listener};
  processor.ProcessEvents(response.capture_events());
}