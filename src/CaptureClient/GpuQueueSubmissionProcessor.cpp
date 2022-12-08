// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/GpuQueueSubmissionProcessor.h"

#include <absl/meta/type_traits.h>
#include <absl/strings/numbers.h>
#include <stddef.h>

#include <tuple>
#include <utility>

#include "OrbitBase/Logging.h"

namespace orbit_capture_client {

using orbit_client_protos::Color;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::GpuCommandBuffer;
using orbit_grpc_protos::GpuJob;
using orbit_grpc_protos::GpuQueueSubmission;

std::vector<TimerInfo> GpuQueueSubmissionProcessor::ProcessGpuQueueSubmission(
    const GpuQueueSubmission& gpu_queue_submission,
    const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool,
    const std::function<uint64_t(std::string_view str)>&
        get_string_hash_and_send_to_listener_if_necessary) {
  uint32_t thread_id = gpu_queue_submission.meta_info().tid();
  uint64_t pre_submission_cpu_timestamp =
      gpu_queue_submission.meta_info().pre_submission_cpu_timestamp();
  uint64_t post_submission_cpu_timestamp =
      gpu_queue_submission.meta_info().post_submission_cpu_timestamp();
  const GpuJob* matching_gpu_job =
      FindMatchingGpuJob(thread_id, pre_submission_cpu_timestamp, post_submission_cpu_timestamp);

  // If we haven't found the matching "GpuJob" or the submission contains "begin" markers (which
  // might have the "end" markers in a later submission), we save the "GpuSubmission" for later.
  // Note that as soon as all "begin" markers have been processed, the "GpuSubmission" will be
  // deleted again.
  if (matching_gpu_job == nullptr || gpu_queue_submission.num_begin_markers() > 0) {
    tid_to_post_submission_time_to_gpu_submission_[thread_id][post_submission_cpu_timestamp] =
        gpu_queue_submission;
  }
  if (gpu_queue_submission.num_begin_markers() > 0) {
    tid_to_post_submission_time_to_num_begin_markers_[thread_id][post_submission_cpu_timestamp] =
        gpu_queue_submission.num_begin_markers();
  }
  if (matching_gpu_job == nullptr) {
    return {};
  }

  // Save the timestamp now, as after the call to `ProcessGpuQueueSubmissionWithMatchingGpuJob`,
  // the matching_gpu_job may already be deleted.
  uint64_t submission_cpu_timestamp = matching_gpu_job->amdgpu_cs_ioctl_time_ns();

  std::vector<TimerInfo> result = ProcessGpuQueueSubmissionWithMatchingGpuJob(
      gpu_queue_submission, *matching_gpu_job, string_intern_pool,
      get_string_hash_and_send_to_listener_if_necessary);

  if (!HasUnprocessedBeginMarkers(thread_id, post_submission_cpu_timestamp)) {
    DeleteSavedGpuJob(thread_id, submission_cpu_timestamp);
  }
  return result;
}
std::vector<orbit_client_protos::TimerInfo> GpuQueueSubmissionProcessor::ProcessGpuJob(
    const GpuJob& gpu_job, const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool,
    const std::function<uint64_t(std::string_view str)>&
        get_string_hash_and_send_to_listener_if_necessary) {
  uint32_t thread_id = gpu_job.tid();
  uint64_t amdgpu_cs_ioctl_time_ns = gpu_job.amdgpu_cs_ioctl_time_ns();
  const GpuQueueSubmission* matching_gpu_submission =
      FindMatchingGpuQueueSubmission(thread_id, amdgpu_cs_ioctl_time_ns);

  // If we haven't found the matching "GpuSubmission" or the submission contains "begin" markers
  // (which might have the "end" markers in a later submission), we save the "GpuJob" for later.
  // Note that as soon as all "begin" markers have been processed, the "GpuJob" will be deleted
  // again.
  if (matching_gpu_submission == nullptr || matching_gpu_submission->num_begin_markers() > 0) {
    tid_to_submission_time_to_gpu_job_[thread_id][amdgpu_cs_ioctl_time_ns] = gpu_job;
  }
  if (matching_gpu_submission == nullptr) {
    return {};
  }

  uint64_t post_submission_cpu_timestamp =
      matching_gpu_submission->meta_info().post_submission_cpu_timestamp();

  std::vector<TimerInfo> result = ProcessGpuQueueSubmissionWithMatchingGpuJob(
      *matching_gpu_submission, gpu_job, string_intern_pool,
      get_string_hash_and_send_to_listener_if_necessary);

  if (!HasUnprocessedBeginMarkers(thread_id, post_submission_cpu_timestamp)) {
    DeleteSavedGpuSubmission(thread_id, post_submission_cpu_timestamp);
  }
  return result;
}

const GpuQueueSubmission* GpuQueueSubmissionProcessor::FindMatchingGpuQueueSubmission(
    uint32_t thread_id, uint64_t submit_time) {
  const auto& post_submission_time_to_gpu_submission_it =
      tid_to_post_submission_time_to_gpu_submission_.find(thread_id);
  if (post_submission_time_to_gpu_submission_it ==
      tid_to_post_submission_time_to_gpu_submission_.end()) {
    return nullptr;
  }

  const auto& post_submission_time_to_gpu_submission =
      post_submission_time_to_gpu_submission_it->second;

  // Find the first Gpu submission with a "post submission" timestamp greater or equal to the Gpu
  // job's timestamp. If the "pre submission" timestamp is not greater (i.e. less or equal) than the
  // job's timestamp, we have found the matching submission.
  auto lower_bound_gpu_submission_it =
      post_submission_time_to_gpu_submission.lower_bound(submit_time);
  if (lower_bound_gpu_submission_it == post_submission_time_to_gpu_submission.end()) {
    return nullptr;
  }
  const GpuQueueSubmission* matching_gpu_submission = &lower_bound_gpu_submission_it->second;

  if (matching_gpu_submission->meta_info().pre_submission_cpu_timestamp() > submit_time) {
    return nullptr;
  }

  return matching_gpu_submission;
}

const GpuJob* GpuQueueSubmissionProcessor::FindMatchingGpuJob(
    uint32_t thread_id, uint64_t pre_submission_cpu_timestamp,
    uint64_t post_submission_cpu_timestamp) {
  const auto& submission_time_to_gpu_job_it = tid_to_submission_time_to_gpu_job_.find(thread_id);
  if (submission_time_to_gpu_job_it == tid_to_submission_time_to_gpu_job_.end()) {
    return nullptr;
  }

  const auto& submission_time_to_gpu_job = submission_time_to_gpu_job_it->second;

  // Find the first Gpu job that has a timestamp greater or equal to the "pre submission" timestamp:
  auto gpu_job_matching_pre_submission_it =
      submission_time_to_gpu_job.lower_bound(pre_submission_cpu_timestamp);
  if (gpu_job_matching_pre_submission_it == submission_time_to_gpu_job.end()) {
    return nullptr;
  }

  // Find the first Gpu job that has a timestamp greater to the "post submission" timestamp
  // (which would be the next job) and decrease the iterator by one.
  auto gpu_job_matching_post_submission_it =
      submission_time_to_gpu_job.upper_bound(post_submission_cpu_timestamp);
  if (gpu_job_matching_post_submission_it == submission_time_to_gpu_job.begin()) {
    return nullptr;
  }
  --gpu_job_matching_post_submission_it;

  if (&gpu_job_matching_pre_submission_it->second != &gpu_job_matching_post_submission_it->second) {
    return nullptr;
  }

  return &gpu_job_matching_pre_submission_it->second;
}

std::vector<TimerInfo> GpuQueueSubmissionProcessor::ProcessGpuQueueSubmissionWithMatchingGpuJob(
    const GpuQueueSubmission& gpu_queue_submission, const GpuJob& matching_gpu_job,
    const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool,
    const std::function<uint64_t(std::string_view str)>&
        get_string_hash_and_send_to_listener_if_necessary) {
  std::vector<TimerInfo> result;
  uint64_t timeline_key = matching_gpu_job.timeline_key();
  ORBIT_CHECK(string_intern_pool.contains(timeline_key));
  std::string timeline = string_intern_pool.at(timeline_key);

  std::optional<GpuCommandBuffer> first_command_buffer =
      ExtractFirstCommandBuffer(gpu_queue_submission);

  // The first command buffer acts as our reference needed to align GPU time based events in the
  // CPU timeline. If we are missing the first timestamp of the submission -- which is the case if
  // we started capturing within its execution -- we need to discard the submission.
  if (first_command_buffer.has_value() && first_command_buffer->begin_gpu_timestamp_ns() == 0) {
    return result;
  }

  std::vector<TimerInfo> command_buffer_timers =
      ProcessGpuCommandBuffers(gpu_queue_submission, matching_gpu_job, first_command_buffer,
                               timeline_key, get_string_hash_and_send_to_listener_if_necessary);

  result.insert(result.end(), command_buffer_timers.begin(), command_buffer_timers.end());

  std::vector<TimerInfo> debug_marker_timers = ProcessGpuDebugMarkers(
      gpu_queue_submission, matching_gpu_job, first_command_buffer, string_intern_pool);

  result.insert(result.end(), debug_marker_timers.begin(), debug_marker_timers.end());

  return result;
}

bool GpuQueueSubmissionProcessor::HasUnprocessedBeginMarkers(
    uint32_t thread_id, uint64_t post_submission_timestamp) const {
  if (!tid_to_post_submission_time_to_num_begin_markers_.contains(thread_id)) {
    return false;
  }
  if (!tid_to_post_submission_time_to_num_begin_markers_.at(thread_id).contains(
          post_submission_timestamp)) {
    return false;
  }
  ORBIT_CHECK(tid_to_post_submission_time_to_num_begin_markers_.at(thread_id).at(
                  post_submission_timestamp) > 0);
  return true;
}

void GpuQueueSubmissionProcessor::DecrementUnprocessedBeginMarkers(
    uint32_t thread_id, uint64_t submission_timestamp, uint64_t post_submission_timestamp) {
  ORBIT_CHECK(tid_to_post_submission_time_to_num_begin_markers_.contains(thread_id));
  auto& post_submission_time_to_num_begin_markers =
      tid_to_post_submission_time_to_num_begin_markers_.at(thread_id);
  ORBIT_CHECK(post_submission_time_to_num_begin_markers.contains(post_submission_timestamp));
  uint64_t new_num = post_submission_time_to_num_begin_markers.at(post_submission_timestamp) - 1;
  post_submission_time_to_num_begin_markers.at(post_submission_timestamp) = new_num;
  if (new_num == 0) {
    post_submission_time_to_num_begin_markers.erase(post_submission_timestamp);
    if (post_submission_time_to_num_begin_markers.empty()) {
      tid_to_post_submission_time_to_num_begin_markers_.erase(thread_id);
      DeleteSavedGpuJob(thread_id, submission_timestamp);
      DeleteSavedGpuSubmission(thread_id, post_submission_timestamp);
    }
  }
}

void GpuQueueSubmissionProcessor::DeleteSavedGpuJob(uint32_t thread_id,
                                                    uint64_t submission_timestamp) {
  if (!tid_to_submission_time_to_gpu_job_.contains(thread_id)) {
    return;
  }
  // This method might be called even when the "capture start" falls directly inside a GpuJob, and
  // we thus don't have the job present in the map.
  // For simplicity we "erase" it anyways (insert and remove it again).
  auto& submission_time_to_gpu_job = tid_to_submission_time_to_gpu_job_[thread_id];
  submission_time_to_gpu_job.erase(submission_timestamp);
  if (submission_time_to_gpu_job.empty()) {
    tid_to_submission_time_to_gpu_job_.erase(thread_id);
  }
}
void GpuQueueSubmissionProcessor::DeleteSavedGpuSubmission(uint32_t thread_id,
                                                           uint64_t post_submission_timestamp) {
  if (!tid_to_post_submission_time_to_gpu_submission_.contains(thread_id)) {
    return;
  }
  auto& post_submission_time_to_gpu_submission =
      tid_to_post_submission_time_to_gpu_submission_.at(thread_id);
  post_submission_time_to_gpu_submission.erase(post_submission_timestamp);
  if (post_submission_time_to_gpu_submission.empty()) {
    tid_to_post_submission_time_to_gpu_submission_.erase(thread_id);
  }
}

std::vector<TimerInfo> GpuQueueSubmissionProcessor::ProcessGpuCommandBuffers(
    const GpuQueueSubmission& gpu_queue_submission, const GpuJob& matching_gpu_job,
    const std::optional<GpuCommandBuffer>& first_command_buffer, uint64_t timeline_hash,
    const std::function<uint64_t(std::string_view str)>&
        get_string_hash_and_send_to_listener_if_necessary) const {
  constexpr const char* kCommandBufferLabel = "command buffer";
  uint64_t command_buffer_text_key =
      get_string_hash_and_send_to_listener_if_necessary(kCommandBufferLabel);

  uint32_t thread_id = gpu_queue_submission.meta_info().tid();
  uint32_t process_id = gpu_queue_submission.meta_info().pid();

  std::vector<TimerInfo> result;

  for (const auto& submit_info : gpu_queue_submission.submit_infos()) {
    for (const auto& command_buffer : submit_info.command_buffers()) {
      ORBIT_CHECK(first_command_buffer != std::nullopt);
      TimerInfo command_buffer_timer;
      if (command_buffer.begin_gpu_timestamp_ns() != 0) {
        command_buffer_timer.set_start(command_buffer.begin_gpu_timestamp_ns() -
                                       first_command_buffer->begin_gpu_timestamp_ns() +
                                       matching_gpu_job.gpu_hardware_start_time_ns());
      } else {
        command_buffer_timer.set_start(begin_capture_time_ns_);
      }

      command_buffer_timer.set_end(command_buffer.end_gpu_timestamp_ns() -
                                   first_command_buffer->begin_gpu_timestamp_ns() +
                                   matching_gpu_job.gpu_hardware_start_time_ns());
      command_buffer_timer.set_depth(matching_gpu_job.depth());
      command_buffer_timer.set_timeline_hash(timeline_hash);
      command_buffer_timer.set_processor(-1);
      command_buffer_timer.set_thread_id(thread_id);
      command_buffer_timer.set_process_id(process_id);
      command_buffer_timer.set_type(TimerInfo::kGpuCommandBuffer);
      command_buffer_timer.set_user_data_key(command_buffer_text_key);
      result.push_back(command_buffer_timer);
    }
  }
  return result;
}

std::vector<TimerInfo> GpuQueueSubmissionProcessor::ProcessGpuDebugMarkers(
    const GpuQueueSubmission& gpu_queue_submission, const GpuJob& matching_gpu_job,
    const std::optional<GpuCommandBuffer>& first_command_buffer,
    const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool) {
  if (gpu_queue_submission.completed_markers_size() == 0) {
    return {};
  }
  std::vector<TimerInfo> result;

  const auto& submission_meta_info = gpu_queue_submission.meta_info();
  const uint32_t submission_thread_id = submission_meta_info.tid();
  const uint32_t submission_process_id = submission_meta_info.pid();
  uint64_t submission_pre_submission_cpu_timestamp =
      submission_meta_info.pre_submission_cpu_timestamp();
  uint64_t submission_post_submission_cpu_timestamp =
      submission_meta_info.post_submission_cpu_timestamp();

  static constexpr int32_t kUnknownThreadId = -1;

  // GpuQueueSubmissions and GpuJobs will be saved if they contain "begin markers"
  // and only ereased again, after all "begin markers" have been processed.
  // The "begin markers" are likely in the same submission as their "end marker",
  // thus will be erased after the last completed marker was processed.
  // This would also erase the current "gpu_queue_submission" if we would do it
  // right away.
  // To prevent this, we do our processing first, collect all "begin markers" to
  // decrement, and decrement them at then very and.
  std::vector<std::tuple<int32_t /*thread_id*/, uint64_t /*submit_time_ns*/,
                         uint64_t /*post_submit_time_ns*/>>
      begin_markers_to_decrement;

  for (const auto& completed_marker : gpu_queue_submission.completed_markers()) {
    ORBIT_CHECK(first_command_buffer != std::nullopt);
    TimerInfo marker_timer;

    // If we've recorded the submission that contains the begin marker, we'll retrieve this
    // submission from our mappings, and set the markers begin time accordingly.
    // Otherwise, we will use the capture start time as begin.
    if (completed_marker.has_begin_marker()) {
      const auto& begin_marker_info = completed_marker.begin_marker();
      const auto& begin_marker_meta_info = begin_marker_info.meta_info();
      const uint32_t begin_marker_thread_id = begin_marker_meta_info.tid();
      uint64_t begin_marker_post_submission_cpu_timestamp =
          begin_marker_meta_info.post_submission_cpu_timestamp();
      uint64_t begin_marker_pre_submission_cpu_timestamp =
          begin_marker_meta_info.pre_submission_cpu_timestamp();

      // Note that the "begin" and "end" of a debug marker may not happen on the same submission.
      // For those cases, we save the meta information of the "begin" marker's submission in the
      // marker information. We will always send the marker on the "end" marker's submission though.
      // So let's check if the meta data is the same as the current submission (i.e. the marker
      // begins and ends on this submission). If this is the case, use that submission. Otherwise,
      // find the submission that matches the given meta data (that we must have received before,
      // and must still be saved).
      std::optional<GpuCommandBuffer> begin_submission_first_command_buffer;
      if (submission_pre_submission_cpu_timestamp == begin_marker_pre_submission_cpu_timestamp &&
          submission_post_submission_cpu_timestamp == begin_marker_post_submission_cpu_timestamp &&
          submission_thread_id == begin_marker_thread_id) {
        begin_submission_first_command_buffer = ExtractFirstCommandBuffer(gpu_queue_submission);
      } else {
        const GpuQueueSubmission* matching_begin_submission = FindMatchingGpuQueueSubmission(
            begin_marker_thread_id, begin_marker_post_submission_cpu_timestamp);
        // Note that we receive submissions of a single queue in order (by CPU submission time).
        // However, if we are out of timer slot indices, we might discard submissions (if it
        // contains no command buffer timers). If we don't have a matching submission for the
        // "begin" marker, we have to discard the entire marker.
        if (matching_begin_submission == nullptr) {
          ORBIT_ERROR("Discarding debug marker timer.");
          continue;
        }
        begin_submission_first_command_buffer =
            ExtractFirstCommandBuffer(*matching_begin_submission);
      }
      ORBIT_CHECK(begin_submission_first_command_buffer.has_value());

      const GpuJob* matching_begin_job = FindMatchingGpuJob(
          begin_marker_thread_id, begin_marker_meta_info.pre_submission_cpu_timestamp(),
          begin_marker_post_submission_cpu_timestamp);

      uint64_t begin_submission_time_ns = 0;
      if (matching_begin_job != nullptr) {
        // Convert the GPU time to CPU time, based on the CPU time of the HW execution begin and the
        // GPU timestamp of the begin of the first command buffer. Note that we will assume that the
        // first command buffer starts execution right away as an approximation.
        marker_timer.set_start(completed_marker.begin_marker().gpu_timestamp_ns() +
                               matching_begin_job->gpu_hardware_start_time_ns() -
                               begin_submission_first_command_buffer->begin_gpu_timestamp_ns());
        begin_submission_time_ns = matching_begin_job->amdgpu_cs_ioctl_time_ns();
      } else {
        // We might have bad luck and have captured the "begin" submission, but not the matching
        // job.
        marker_timer.set_start(begin_capture_time_ns_);
      }

      if (begin_marker_thread_id == gpu_queue_submission.meta_info().tid()) {
        marker_timer.set_thread_id(begin_marker_thread_id);
      } else {
        marker_timer.set_thread_id(kUnknownThreadId);
      }

      // Remember, it would not be safe to decrement (and thus possible erase) the "begin marker"
      // here right away, as its begin submission might be the same as "gpu_queue_submission",
      // which we still use afterwards.
      begin_markers_to_decrement.emplace_back(begin_marker_thread_id, begin_submission_time_ns,
                                              begin_marker_post_submission_cpu_timestamp);
    } else {
      marker_timer.set_start(begin_capture_time_ns_);
      marker_timer.set_thread_id(kUnknownThreadId);
    }

    marker_timer.set_process_id(submission_process_id);
    marker_timer.set_depth(completed_marker.depth());
    marker_timer.set_timeline_hash(matching_gpu_job.timeline_key());
    marker_timer.set_processor(-1);
    marker_timer.set_type(TimerInfo::kGpuDebugMarker);
    marker_timer.set_end(completed_marker.end_gpu_timestamp_ns() -
                         first_command_buffer->begin_gpu_timestamp_ns() +
                         matching_gpu_job.gpu_hardware_start_time_ns());

    if (completed_marker.has_color()) {
      Color* color = marker_timer.mutable_color();
      color->set_red(static_cast<uint32_t>(completed_marker.color().red() * 255.f));
      color->set_green(static_cast<uint32_t>(completed_marker.color().green() * 255.f));
      color->set_blue(static_cast<uint32_t>(completed_marker.color().blue() * 255.f));
      color->set_alpha(static_cast<uint32_t>(completed_marker.color().alpha() * 255.f));
    }

    const uint64_t text_key = completed_marker.text_key();
    marker_timer.set_user_data_key(text_key);

    // We have special handling for DXVK instrumentation that have an encoded group_id in their
    // label.
    ORBIT_CHECK(string_intern_pool.contains(text_key));
    const std::string& text = string_intern_pool.at(text_key);
    uint64_t group_id = 0;
    if (TryExtractDXVKVulkanGroupIdFromDebugLabel(text, &group_id)) {
      marker_timer.set_group_id(group_id);
    }

    result.push_back(marker_timer);
  }

  // Now we are done and can decrement the processed "begin markers".
  for (auto [thread_id, submit_time_ns, post_submit_time_ns] : begin_markers_to_decrement) {
    DecrementUnprocessedBeginMarkers(thread_id, submit_time_ns, post_submit_time_ns);
  }
  return result;
}

std::optional<GpuCommandBuffer> GpuQueueSubmissionProcessor::ExtractFirstCommandBuffer(
    const GpuQueueSubmission& gpu_queue_submission) {
  for (const auto& submit_info : gpu_queue_submission.submit_infos()) {
    for (const auto& command_buffer : submit_info.command_buffers()) {
      return command_buffer;
    }
  }
  return std::nullopt;
}

bool GpuQueueSubmissionProcessor::TryExtractDXVKVulkanGroupIdFromDebugLabel(
    std::string_view label, uint64_t* out_group_id) {
  // We have special handling for DXVK instrumentation that extracts the encoded group_id from
  // the label's text. The encoding is:
  // 'DXVK__vkFunctionName#GROUP_ID', where 'GROUP_ID' is the group id.
  if (label.find_first_of("DXVK__") != std::string::npos) {
    const size_t group_id_index = label.find_last_of('#') + 1;
    if (group_id_index != std::string::npos &&
        absl::SimpleAtoi(label.substr(group_id_index), out_group_id)) {
      return true;
    }
  }
  return false;
}

}  // namespace orbit_capture_client
