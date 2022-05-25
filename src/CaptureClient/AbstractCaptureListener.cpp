// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/AbstractCaptureListener.h"

#include "ClientData/LinuxAddressInfo.h"
#include "ClientData/TracepointInfo.h"

namespace orbit_capture_client {

void AbstractCaptureListener::OnAddressInfo(orbit_client_data::LinuxAddressInfo address_info) {
  GetMutableCaptureData().InsertAddressInfo(std::move(address_info));
}

void AbstractCaptureListener::OnUniqueTracepointInfo(
    uint64_t tracepoint_id, orbit_client_data::TracepointInfo tracepoint_info) {
  GetMutableCaptureData().AddUniqueTracepointInfo(tracepoint_id, std::move(tracepoint_info));
}

void AbstractCaptureListener::OnUniqueCallstack(uint64_t callstack_id,
                                                orbit_client_data::CallstackInfo callstack) {
  GetMutableCaptureData().AddUniqueCallstack(callstack_id, std::move(callstack));
}

void AbstractCaptureListener::OnCallstackEvent(orbit_client_data::CallstackEvent callstack_event) {
  GetMutableCaptureData().AddCallstackEvent(callstack_event);
}

void AbstractCaptureListener::OnThreadName(uint32_t thread_id, std::string thread_name) {
  GetMutableCaptureData().AddOrAssignThreadName(thread_id, std::move(thread_name));
}

void AbstractCaptureListener::OnThreadStateSlice(
    orbit_client_data::ThreadStateSliceInfo thread_state_slice) {
  GetMutableCaptureData().AddThreadStateSlice(thread_state_slice);
}

void AbstractCaptureListener::OnTracepointEvent(
    orbit_client_data::TracepointEventInfo tracepoint_event_info) {
  uint32_t capture_process_id = GetMutableCaptureData().process_id();
  bool is_same_pid_as_target = capture_process_id == tracepoint_event_info.pid();

  GetMutableCaptureData().AddTracepointEventAndMapToThreads(
      tracepoint_event_info.timestamp_ns(), tracepoint_event_info.tracepoint_id(),
      tracepoint_event_info.pid(), tracepoint_event_info.tid(), tracepoint_event_info.cpu(),
      is_same_pid_as_target);
}

}  // namespace orbit_capture_client