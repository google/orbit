// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CAPTURE_CLIENT_CAPTURE_LISTENER_H_
#define ORBIT_CAPTURE_CLIENT_CAPTURE_LISTENER_H_

#include "../../OrbitGl/TracepointCustom.h"
#include "Callstack.h"
#include "EventBuffer.h"
#include "OrbitProcess.h"
#include "ScopeTimer.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "capture_data.pb.h"

class CaptureListener {
 public:
  virtual ~CaptureListener() = default;

  // Called after capture started but before the first event arrived.
  virtual void OnCaptureStarted(
      int32_t process_id, std::string process_name, std::shared_ptr<Process> process,
      absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions,
      absl::flat_hash_set<orbit_grpc_protos::TracepointInfo, HashTracepointInfo,
                          EqualTracepointInfo>
          selected_tracepoints) = 0;
  // Called when capture is complete
  virtual void OnCaptureComplete() = 0;

  virtual void OnTimer(const orbit_client_protos::TimerInfo& timer_info) = 0;
  virtual void OnKeyAndString(uint64_t key, std::string str) = 0;
  virtual void OnUniqueCallStack(CallStack callstack) = 0;
  virtual void OnCallstackEvent(orbit_client_protos::CallstackEvent callstack_event) = 0;
  virtual void OnThreadName(int32_t thread_id, std::string thread_name) = 0;
  virtual void OnAddressInfo(orbit_client_protos::LinuxAddressInfo address_info) = 0;
};

#endif  // ORBIT_GL_CAPTURE_LISTENER_H_
