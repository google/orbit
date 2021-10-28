// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TrackTestData.h"

using orbit_client_data::CaptureData;

namespace orbit_gl {

std::unique_ptr<CaptureData> TrackTestData::GenerateTestCaptureData() {
  auto capture_data = std::make_unique<CaptureData>(nullptr, orbit_grpc_protos::CaptureStarted{},
                                                    std::nullopt, absl::flat_hash_set<uint64_t>{},
                                                    CaptureData::DataSource::kLiveCapture);

  // AddressInfo
  orbit_client_protos::LinuxAddressInfo address_info;
  address_info.set_absolute_address(kInstructionAbsoluteAddress);
  address_info.set_offset_in_function(kInstructionAbsoluteAddress - kFunctionAbsoluteAddress);
  address_info.set_function_name(kFunctionName);
  address_info.set_module_path(kModuleName);
  capture_data->InsertAddressInfo(address_info);

  // CallstackInfo
  const std::vector<uint64_t> callstack_frames{kInstructionAbsoluteAddress};
  orbit_client_protos::CallstackInfo callstack_info;
  *callstack_info.mutable_frames() = {callstack_frames.begin(), callstack_frames.end()};
  callstack_info.set_type(orbit_client_protos::CallstackInfo_CallstackType_kComplete);
  capture_data->AddUniqueCallstack(kCallstackId, std::move(callstack_info));

  // CallstackEvent
  orbit_client_protos::CallstackEvent callstack_event;
  callstack_event.set_callstack_id(kCallstackId);
  callstack_event.set_thread_id(kThreadId);
  capture_data->AddCallstackEvent(std::move(callstack_event));

  capture_data->AddOrAssignThreadName(kThreadId, kThreadName);
  capture_data->AddOrAssignThreadName(kTimerOnlyThreadId, kTimerOnlyThreadName);

  return capture_data;
}

}  // namespace orbit_gl