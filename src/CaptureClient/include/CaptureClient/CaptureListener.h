// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_CAPTURE_LISTENER_H_
#define CAPTURE_CLIENT_CAPTURE_LISTENER_H_

#include "ClientData/Callstack.h"
#include "ClientData/ProcessData.h"
#include "ClientData/TracepointCustom.h"
#include "ClientData/UserDefinedCaptureData.h"
#include "OrbitBase/Result.h"
#include "absl/container/flat_hash_set.h"
#include "capture.pb.h"
#include "capture_data.pb.h"

namespace orbit_capture_client {

class CaptureListener {
 public:
  enum class CaptureOutcome { kComplete, kCancelled };

  virtual ~CaptureListener() = default;

  virtual void OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& capture_started,
                                absl::flat_hash_set<uint64_t> frame_track_function_ids) = 0;
  virtual void OnCaptureFinished(const orbit_grpc_protos::CaptureFinished& capture_finished) = 0;

  virtual void OnTimer(const orbit_client_protos::TimerInfo& timer_info) = 0;
  virtual void OnSystemMemoryUsage(
      const orbit_grpc_protos::SystemMemoryUsage& system_memory_usage) = 0;
  virtual void OnKeyAndString(uint64_t key, std::string str) = 0;
  virtual void OnUniqueCallStack(CallStack callstack) = 0;
  virtual void OnCallstackEvent(orbit_client_protos::CallstackEvent callstack_event) = 0;
  virtual void OnThreadName(int32_t thread_id, std::string thread_name) = 0;
  virtual void OnModuleUpdate(uint64_t timestamp_ns, orbit_grpc_protos::ModuleInfo module_info) = 0;
  virtual void OnModulesSnapshot(uint64_t timestamp_ns,
                                 std::vector<orbit_grpc_protos::ModuleInfo> module_infos) = 0;
  virtual void OnThreadStateSlice(orbit_client_protos::ThreadStateSliceInfo thread_state_slice) = 0;
  virtual void OnAddressInfo(orbit_client_protos::LinuxAddressInfo address_info) = 0;
  virtual void OnUniqueTracepointInfo(uint64_t key,
                                      orbit_grpc_protos::TracepointInfo tracepoint_info) = 0;
  virtual void OnTracepointEvent(
      orbit_client_protos::TracepointEventInfo tracepoint_event_info) = 0;
};

}  // namespace orbit_capture_client

#endif  // CAPTURE_CLIENT_CAPTURE_LISTENER_H_
