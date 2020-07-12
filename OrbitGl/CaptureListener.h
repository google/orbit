// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_LISTENER_H_
#define ORBIT_GL_CAPTURE_LISTENER_H_

#include "EventBuffer.h"
#include "KeyAndString.h"
#include "ScopeTimer.h"
#include "capture.pb.h"

class CaptureListener {
 public:
  virtual ~CaptureListener() = default;
  virtual void OnTimer(Timer timer) = 0;
  virtual void OnKeyAndString(uint64_t key, std::string str) = 0;
  virtual void OnCallstack(Callstack callstack) = 0;
  virtual void OnCallstackEvent(CallstackEvent callstack_event) = 0;
  virtual void OnThreadName(int32_t thread_id, std::string thread_name) = 0;
  virtual void OnAddressInfo(AddressInfo address_info) = 0;
};

#endif  // ORBIT_GL_CAPTURE_LISTENER_H_
