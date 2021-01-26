// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CAPTURE_CLIENT_CAPTURE_EVENT_PROCESSOR_H_
#define ORBIT_CAPTURE_CLIENT_CAPTURE_EVENT_PROCESSOR_H_

#include <absl/container/flat_hash_set.h>

#include <cstdint>
#include <functional>
#include <string>

#include "OrbitCaptureClient/ApiEventProcessor.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "OrbitCaptureClient/GpuQueueSubmissionProcessor.h"
#include "capture.pb.h"
#include "services.pb.h"
#include "tracepoint.pb.h"

class CaptureEventProcessor {
 public:
  explicit CaptureEventProcessor(CaptureListener* capture_listener)
      : capture_listener_(capture_listener), api_event_processor_(capture_listener) {}

  void ProcessEvent(const orbit_grpc_protos::ClientCaptureEvent& event);

  template <typename Iterable>
  void ProcessEvents(const Iterable& events) {
    for (const auto& event : events) {
      ProcessEvent(event);
    }
  }

 private:
  void ProcessSchedulingSlice(const orbit_grpc_protos::SchedulingSlice& scheduling_slice);
  void ProcessInternedCallstack(orbit_grpc_protos::InternedCallstack interned_callstack);
  void ProcessCallstackSample(const orbit_grpc_protos::CallstackSample& callstack_sample);
  void ProcessFunctionCall(const orbit_grpc_protos::FunctionCall& function_call);
  void ProcessIntrospectionScope(const orbit_grpc_protos::IntrospectionScope& introspection_scope);
  void ProcessInternedString(orbit_grpc_protos::InternedString interned_string);
  void ProcessGpuJob(const orbit_grpc_protos::GpuJob& gpu_job);
  void ProcessThreadName(const orbit_grpc_protos::ThreadName& thread_name);
  void ProcessThreadStateSlice(const orbit_grpc_protos::ThreadStateSlice& thread_state_slice);
  void ProcessAddressInfo(const orbit_grpc_protos::AddressInfo& address_info);
  void ProcessInternedTracepointInfo(
      orbit_grpc_protos::InternedTracepointInfo interned_tracepoint_info);
  void ProcessTracepointEvent(const orbit_grpc_protos::TracepointEvent& tracepoint_event);
  void ProcessGpuQueueSubmission(const orbit_grpc_protos::GpuQueueSubmission& gpu_command_buffer);

  absl::flat_hash_map<uint64_t, orbit_grpc_protos::Callstack> callstack_intern_pool;
  absl::flat_hash_map<uint64_t, std::string> string_intern_pool_;
  CaptureListener* capture_listener_ = nullptr;

  absl::flat_hash_set<uint64_t> callstack_hashes_seen_;
  void SendCallstackToListenerIfNecessary(uint64_t callstack_id,
                                          const orbit_grpc_protos::Callstack& callstack);
  absl::flat_hash_set<uint64_t> string_hashes_seen_;
  uint64_t GetStringHashAndSendToListenerIfNecessary(const std::string& str);

  GpuQueueSubmissionProcessor gpu_queue_submission_processor_;
  ApiEventProcessor api_event_processor_;
};

#endif  // ORBIT_GL_CAPTURE_EVENT_PROCESSOR_H_
