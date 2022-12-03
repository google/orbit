// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/TrackTestData.h"

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>

#include <optional>
#include <string>
#include <utility>

#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"
#include "ClientData/LinuxAddressInfo.h"
#include "GrpcProtos/capture.pb.h"

using orbit_client_data::CallstackType;
using orbit_client_data::CaptureData;
using orbit_client_data::LinuxAddressInfo;

namespace orbit_gl {

std::unique_ptr<CaptureData> TrackTestData::GenerateTestCaptureData() {
  orbit_grpc_protos::CaptureStarted capture_started;
  orbit_grpc_protos::InstrumentedFunction* func =
      capture_started.mutable_capture_options()->mutable_instrumented_functions()->Add();
  func->set_function_id(kFunctionId);
  auto capture_data =
      std::make_unique<CaptureData>(capture_started, std::nullopt, absl::flat_hash_set<uint64_t>{},
                                    CaptureData::DataSource::kLiveCapture);

  // AddressInfo
  LinuxAddressInfo address_info{kInstructionAbsoluteAddress,
                                kInstructionAbsoluteAddress - kFunctionAbsoluteAddress, kModuleName,
                                kModuleName};

  capture_data->InsertAddressInfo(std::move(address_info));

  // CallstackInfo
  const std::vector<uint64_t> callstack_frames{kInstructionAbsoluteAddress};
  orbit_client_data::CallstackInfo callstack_info{
      {callstack_frames.begin(), callstack_frames.end()}, CallstackType::kComplete};
  capture_data->AddUniqueCallstack(kCallstackId, std::move(callstack_info));

  // CallstackEvent
  orbit_client_data::CallstackEvent callstack_event0{1234, kCallstackId, kThreadId};
  capture_data->AddCallstackEvent(callstack_event0);

  orbit_client_data::CallstackEvent callstack_event1{5000, kCallstackId, kThreadId};
  capture_data->AddCallstackEvent(callstack_event1);

  capture_data->AddOrAssignThreadName(kThreadId, kThreadName);
  capture_data->AddOrAssignThreadName(kTimerOnlyThreadId, kTimerOnlyThreadName);

  return capture_data;
}

std::vector<orbit_client_protos::TimerInfo> TrackTestData::GenerateTimers() {
  using orbit_client_protos::TimerInfo;

  TimerInfo timer;
  timer.set_start(0);
  timer.set_end(100);
  timer.set_thread_id(TrackTestData::kThreadId);
  timer.set_processor(0);
  timer.set_depth(0);
  timer.set_type(TimerInfo::kCoreActivity);

  return {timer};
}

}  // namespace orbit_gl