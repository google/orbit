// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CAPTURE_CLIENT_CAPTURE_EVENT_PROCESSOR_H_
#define ORBIT_CAPTURE_CLIENT_CAPTURE_EVENT_PROCESSOR_H_

#include <cstdint>
#include <string>

#include "OrbitCaptureClient/CaptureListener.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "capture.pb.h"
#include "services.pb.h"
#include "tracepoint.pb.h"

class CaptureEventProcessor {
 public:
  explicit CaptureEventProcessor(CaptureListener* capture_listener)
      : capture_listener_(capture_listener) {}

  void ProcessEvent(const orbit_grpc_protos::CaptureEvent& event);

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

  // Vulkan Layer related helpers:
  void ProcessGpuQueueSubmissionWithMatchingGpuJob(
      const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission,
      const orbit_grpc_protos::GpuJob& matching_gpu_job);
  void ProcessGpuCommandBuffers(
      const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission,
      const orbit_grpc_protos::GpuJob& matching_gpu_job,
      const std::optional<orbit_grpc_protos::GpuCommandBuffer>& first_command_buffer,
      uint64_t timeline_hash);
  void ProcessGpuDebugMarkers(
      const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission,
      const orbit_grpc_protos::GpuJob& matching_gpu_job,
      const std::optional<orbit_grpc_protos::GpuCommandBuffer>& first_command_buffer,
      const std::string& timeline);
  static std::optional<orbit_grpc_protos::GpuCommandBuffer> ExtractFirstCommandBuffer(
      const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission);
  const orbit_grpc_protos::GpuJob* FindMatchingGpuJob(int32_t thread_id,
                                                      uint64_t pre_submission_cpu_timestamp,
                                                      uint64_t post_submission_cpu_timestamp);
  const orbit_grpc_protos::GpuQueueSubmission* FindMatchingGpuQueueSubmission(int32_t thread_id,
                                                                              uint64_t submit_time);
  [[nodiscard]] bool HasUnprocessedBeginMarkers(int32_t thread_id,
                                                uint64_t post_submission_timestamp) const;
  void DecrementUnprocessedBeginMarkers(int32_t thread_id, uint64_t submission_timestamp,
                                        uint64_t post_submission_timestamp);
  void DeleteSavedGpuJob(int32_t thread_id, uint64_t submission_timestamp);
  void DeleteSavedGpuSubmission(int32_t thread_id, uint64_t post_submission_timestamp);

  absl::flat_hash_map<uint64_t, orbit_grpc_protos::Callstack> callstack_intern_pool;
  absl::flat_hash_map<uint64_t, std::string> string_intern_pool_;
  absl::flat_hash_map<uint64_t, orbit_grpc_protos::TracepointInfo> tracepoint_intern_pool_;
  CaptureListener* capture_listener_ = nullptr;

  absl::flat_hash_set<uint64_t> callstack_hashes_seen_;
  uint64_t GetCallstackHashAndSendToListenerIfNecessary(
      const orbit_grpc_protos::Callstack& callstack);
  absl::flat_hash_set<uint64_t> string_hashes_seen_;
  uint64_t GetStringHashAndSendToListenerIfNecessary(const std::string& str);
  absl::flat_hash_set<uint64_t> tracepoint_hashes_seen_;
  void SendTracepointInfoToListenerIfNecessary(
      const orbit_grpc_protos::TracepointInfo& tracepoint_info, const uint64_t& hash);

  absl::flat_hash_map<int32_t, std::map<uint64_t, orbit_grpc_protos::GpuJob>>
      tid_to_submission_time_to_gpu_job_;
  absl::flat_hash_map<int32_t, std::map<uint64_t, orbit_grpc_protos::GpuQueueSubmission>>
      tid_to_post_submission_time_to_gpu_submission_;
  absl::flat_hash_map<int32_t, absl::flat_hash_map<uint64_t, uint32_t>>
      tid_to_post_submission_time_to_num_begin_markers_;
  uint64_t begin_capture_time_ns_ = std::numeric_limits<uint64_t>::max();
};

#endif  // ORBIT_GL_CAPTURE_EVENT_PROCESSOR_H_
