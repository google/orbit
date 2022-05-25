// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_ABSTRACT_CAPTURE_LISTENER_H_
#define CAPTURE_CLIENT_ABSTRACT_CAPTURE_LISTENER_H_

#include "CaptureClient/CaptureListener.h"
#include "ClientData/CaptureData.h"

namespace orbit_capture_client {

class AbstractCaptureListener : public CaptureListener {
 public:
  void OnAddressInfo(orbit_client_data::LinuxAddressInfo address_info) override;
  void OnUniqueTracepointInfo(uint64_t tracepoint_id,
                              orbit_client_data::TracepointInfo tracepoint_info) override;
  void OnUniqueCallstack(uint64_t callstack_id,
                         orbit_client_data::CallstackInfo callstack) override;
  void OnCallstackEvent(orbit_client_data::CallstackEvent callstack_event) override;
  void OnThreadName(uint32_t thread_id, std::string thread_name) override;
  void OnThreadStateSlice(orbit_client_data::ThreadStateSliceInfo thread_state_slice) override;
  void OnTracepointEvent(orbit_client_data::TracepointEventInfo tracepoint_event_info) override;

 protected:
  // TODO(b/166767590): This is mostly written during capture by the capture thread on the
  // CaptureListener parts of App, but may be read also during capturing by all threads.
  // Currently, it is not properly synchronized (and thus it can't live at DataManager).
  std::unique_ptr<orbit_client_data::CaptureData> capture_data_;

 private:
  [[nodiscard]] orbit_client_data::CaptureData& GetMutableCaptureData() {
    ORBIT_CHECK(capture_data_ != nullptr);
    return *capture_data_;
  }
};

}  // namespace orbit_capture_client

#endif  // CAPTURE_CLIENT_ABSTRACT_CAPTURE_LISTENER_H_