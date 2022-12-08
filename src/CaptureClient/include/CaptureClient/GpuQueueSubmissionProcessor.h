// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_GPU_QUEUE_SUBMISSION_PROCESSOR_H_
#define CAPTURE_CLIENT_GPU_QUEUE_SUBMISSION_PROCESSOR_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>
#include <absl/hash/hash.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_capture_client {

// `GpuQueueSubmission` proto objects come with GPU timestamps (rather than CPU timestamps) for the
// command buffer and debug marker timings. In order convert those timestamps into CPU time, each
// `GpuQueueSubmission` contains a timestamp before (pre) and after (post) the vkQueueSubmit driver
// call. For the driver we already have timestamps in the `GpuJob` events. Together with the thread
// id, this allows us to establish a 1:1 mapping of `GpuJob`s and `GpuQueueSubmission`s.
//
// This class allows to convert `GpuQueueSubmission` events (with GPU timestmaps) to command buffer
// and debug marker `TimerInfo`s (with CPU timestamps). For that, it manages the mapping from
// `GpuQueueSubmission`s to their `GpuJob`s and stores those events until not needed anymore.
//
// Worth mentioning is the case of debug markers, where the "begin" marker originates from a
// different submission than the "end" marker. In this case we store the "begin" marker's
// `GpuQueueSubmission` and `GpuJob` until we have processed all corresponding "end" markers.
class GpuQueueSubmissionProcessor {
 public:
  // If the matching `GpuJob` has already been processed, it converts the command buffer and debug
  // marker information from the `GpuQueueSubmission` event into `TimerInfo`s. Otherwise, it
  // returns an empty vector and stores the submission for later processing.
  [[nodiscard]] std::vector<orbit_client_protos::TimerInfo> ProcessGpuQueueSubmission(
      const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission,
      const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool,
      const std::function<uint64_t(std::string_view str)>&
          get_string_hash_and_send_to_listener_if_necessary);

  // If the matching `GpuQueueSubmission` has already been processed, it converts the
  // command buffer and debug marker information from this `GpuQueueSubmission` event into
  // `TimerInfo`s. Otherwise, it returns an empty vector and stores the `GpuJob for later
  // processing.
  [[nodiscard]] std::vector<orbit_client_protos::TimerInfo> ProcessGpuJob(
      const orbit_grpc_protos::GpuJob& gpu_job,
      const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool,
      const std::function<uint64_t(std::string_view str)>&
          get_string_hash_and_send_to_listener_if_necessary);

  // In case we have recorded the submission containing the "begin" of a certain debug marker, we
  // use the `begin_capture_time_ns_` as an approximation for the begin CPU timestamp.
  // This method updates this timestamp with the minimum of the current value and the given value.
  void UpdateBeginCaptureTime(uint64_t timestamp) {
    begin_capture_time_ns_ = std::min(begin_capture_time_ns_, timestamp);
  }

  // We have a special encoding for "group ids" in DXVK Vulkan labels. The encoding is:
  // 'DXVK__vkFunctionName#GROUP_ID', where 'GROUP_ID' is the group id.
  // This function tries to extract the "group id" from the given label, based on this encoding.
  // It returns `true` on success. In this case, the group id will be written to `out_group_id`.
  // In all other cases `false` will be returned.
  static bool TryExtractDXVKVulkanGroupIdFromDebugLabel(std::string_view label,
                                                        uint64_t* out_group_id);

 private:
  [[nodiscard]] std::vector<orbit_client_protos::TimerInfo>
  ProcessGpuQueueSubmissionWithMatchingGpuJob(
      const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission,
      const orbit_grpc_protos::GpuJob& matching_gpu_job,
      const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool,
      const std::function<uint64_t(std::string_view str)>&
          get_string_hash_and_send_to_listener_if_necessary);

  [[nodiscard]] std::vector<orbit_client_protos::TimerInfo> ProcessGpuCommandBuffers(
      const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission,
      const orbit_grpc_protos::GpuJob& matching_gpu_job,
      const std::optional<orbit_grpc_protos::GpuCommandBuffer>& first_command_buffer,
      uint64_t timeline_hash,
      const std::function<uint64_t(std::string_view str)>&
          get_string_hash_and_send_to_listener_if_necessary) const;

  [[nodiscard]] std::vector<orbit_client_protos::TimerInfo> ProcessGpuDebugMarkers(
      const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission,
      const orbit_grpc_protos::GpuJob& matching_gpu_job,
      const std::optional<orbit_grpc_protos::GpuCommandBuffer>& first_command_buffer,
      const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool);

  [[nodiscard]] static std::optional<orbit_grpc_protos::GpuCommandBuffer> ExtractFirstCommandBuffer(
      const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission);

  // Finds the GpuJob that is fully inside the given timestamps and happened on the given thread id.
  // Returns `nullptr` if there is no such job.
  [[nodiscard]] const orbit_grpc_protos::GpuJob* FindMatchingGpuJob(
      uint32_t thread_id, uint64_t pre_submission_cpu_timestamp,
      uint64_t post_submission_cpu_timestamp);

  // Finds the GpuQueueSubmission that fully contains the given timestamp and happened on the given
  // thread id. Returns `nullptr` if there is no such submission.
  [[nodiscard]] const orbit_grpc_protos::GpuQueueSubmission* FindMatchingGpuQueueSubmission(
      uint32_t thread_id, uint64_t submit_time);

  [[nodiscard]] bool HasUnprocessedBeginMarkers(uint32_t thread_id,
                                                uint64_t post_submission_timestamp) const;

  void DecrementUnprocessedBeginMarkers(uint32_t thread_id, uint64_t submission_timestamp,
                                        uint64_t post_submission_timestamp);

  void DeleteSavedGpuJob(uint32_t thread_id, uint64_t submission_timestamp);

  void DeleteSavedGpuSubmission(uint32_t thread_id, uint64_t post_submission_timestamp);

  absl::node_hash_map<int32_t, std::map<uint64_t, orbit_grpc_protos::GpuJob>>
      tid_to_submission_time_to_gpu_job_;
  absl::node_hash_map<int32_t, std::map<uint64_t, orbit_grpc_protos::GpuQueueSubmission>>
      tid_to_post_submission_time_to_gpu_submission_;
  absl::flat_hash_map<int32_t, absl::flat_hash_map<uint64_t, uint32_t>>
      tid_to_post_submission_time_to_num_begin_markers_;

  uint64_t begin_capture_time_ns_ = std::numeric_limits<uint64_t>::max();
};

}  // namespace orbit_capture_client

#endif  // CAPTURE_CLIENT_GPU_QUEUE_SUBMISSION_PROCESSOR_H_
