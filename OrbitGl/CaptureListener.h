// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_LISTENER_H_
#define ORBIT_GL_CAPTURE_LISTENER_H_

#include "Callstack.h"
#include "EventBuffer.h"
#include "KeyAndString.h"
#include "ScopeTimer.h"
#include "capture_data.pb.h"

class CaptureListener {
 public:
  virtual ~CaptureListener() = default;
  virtual void OnTimer(const orbit_client_protos::TimerInfo& timer_info) = 0;
  virtual void OnKeyAndString(uint64_t key, std::string str) = 0;
  virtual void OnCallstack(CallStack callstack) = 0;
  virtual void OnCallstackEvent(
      orbit_client_protos::CallstackEvent callstack_event) = 0;
  virtual void OnThreadName(int32_t thread_id, std::string thread_name) = 0;
  virtual void OnAddressInfo(
      orbit_client_protos::LinuxAddressInfo address_info) = 0;
};

#endif  // ORBIT_GL_CAPTURE_LISTENER_H_
