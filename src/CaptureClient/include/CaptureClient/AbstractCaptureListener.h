// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_ABSTRACT_CAPTURE_LISTENER_H_
#define CAPTURE_CLIENT_ABSTRACT_CAPTURE_LISTENER_H_

#include "CaptureClient/CaptureListener.h"
#include "ClientData/CaptureData.h"

namespace orbit_capture_client {

// TODO(b/234109592) Add a unit-test.
// This is CRTP.  It provides implementations for CaptureListener virtual methods that are shared
// between `OrbitApp` and `MizarData`. `Derived` should implement a public method
// `orbit_client_data::CaptureData& Derived::GetMutableCaptureData()`
template <typename Derived>
class AbstractCaptureListener : public CaptureListener {
 public:
  AbstractCaptureListener() = default;

  AbstractCaptureListener(AbstractCaptureListener&) = default;
  AbstractCaptureListener& operator=(const AbstractCaptureListener& other) = default;

  AbstractCaptureListener(AbstractCaptureListener&&) = default;
  AbstractCaptureListener& operator=(AbstractCaptureListener&& other) = default;

  virtual ~AbstractCaptureListener() = default;

  void OnAddressInfo(orbit_client_data::LinuxAddressInfo address_info) override {
    GetMutableCaptureDataFromDerived().InsertAddressInfo(std::move(address_info));
  }

  void OnUniqueTracepointInfo(uint64_t tracepoint_id,
                              orbit_client_data::TracepointInfo tracepoint_info) override {
    GetMutableCaptureDataFromDerived().AddUniqueTracepointInfo(tracepoint_id,
                                                               std::move(tracepoint_info));
  }

  void OnUniqueCallstack(uint64_t callstack_id,
                         orbit_client_data::CallstackInfo callstack) override {
    GetMutableCaptureDataFromDerived().AddUniqueCallstack(callstack_id, std::move(callstack));
  }

  void OnCallstackEvent(orbit_client_data::CallstackEvent callstack_event) override {
    GetMutableCaptureDataFromDerived().AddCallstackEvent(callstack_event);
  }

  void OnThreadName(uint32_t thread_id, std::string thread_name) override {
    GetMutableCaptureDataFromDerived().AddOrAssignThreadName(thread_id, std::move(thread_name));
  }

  void OnThreadStateSlice(orbit_client_data::ThreadStateSliceInfo thread_state_slice) override {
    GetMutableCaptureDataFromDerived().AddThreadStateSlice(thread_state_slice);
  }

  void OnTracepointEvent(orbit_client_data::TracepointEventInfo tracepoint_event_info) override {
    uint32_t capture_process_id = GetMutableCaptureDataFromDerived().process_id();
    bool is_same_pid_as_target = capture_process_id == tracepoint_event_info.pid();

    GetMutableCaptureDataFromDerived().AddTracepointEventAndMapToThreads(
        tracepoint_event_info.timestamp_ns(), tracepoint_event_info.tracepoint_id(),
        tracepoint_event_info.pid(), tracepoint_event_info.tid(), tracepoint_event_info.cpu(),
        is_same_pid_as_target);
  }

 private:
  orbit_client_data::CaptureData& GetMutableCaptureDataFromDerived() {
    return static_cast<Derived*>(this)->GetMutableCaptureData();
  }
};

}  // namespace orbit_capture_client

#endif  // CAPTURE_CLIENT_ABSTRACT_CAPTURE_LISTENER_H_