// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureClient/GpuQueueSubmissionProcessor.h"

#include "OrbitBase/Logging.h"

using orbit_client_protos::Color;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::GpuCommandBuffer;
using orbit_grpc_protos::GpuDebugMarker;
using orbit_grpc_protos::GpuJob;
using orbit_grpc_protos::GpuQueueSubmission;

std::vector<TimerInfo> GpuQueueSubmissionProcessor::ProcessGpuQueueSubmission(
    const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission,
    const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool,
    const std::function<uint64_t(const std::string& str)>&
        get_string_hash_and_send_to_listener_if_necessary) {
  int32_t thread_id = gpu_queue_submission.meta_info().tid();
  uint64_t pre_submission_cpu_timestamp =
      gpu_queue_submission.meta_info().pre_submission_cpu_timestamp();
  uint64_t post_submission_cpu_timestamp =
      gpu_queue_submission.meta_info().post_submission_cpu_timestamp();
  std::optional<GpuJob> matching_gpu_job_option =
      FindMatchingGpuJob(thread_id, pre_submission_cpu_timestamp, post_submission_cpu_timestamp);

  // If we haven't found the matching "GpuJob" or the submission contains "begin" markers (which
  // might have the "end" markers in a later submission), we save the "GpuSubmission" for later.
  // Note that as soon as all "begin" markers have been processed, the "GpuSubmission" will be
  // deleted again.
  if (!matching_gpu_job_option.has_value() || gpu_queue_submission.num_begin_markers() > 0) {
    tid_to_post_submission_time_to_gpu_submission_[thread_id][post_submission_cpu_timestamp] =
        gpu_queue_submission;
  }
  if (gpu_queue_submission.num_begin_markers() > 0) {
    tid_to_post_submission_time_to_num_begin_markers_[thread_id][post_submission_cpu_timestamp] =
        gpu_queue_submission.num_begin_markers();
  }
  if (!matching_gpu_job_option.has_value()) {
    return {};
  }

  std::vector<TimerInfo> result = ProcessGpuQueueSubmissionWithMatchingGpuJob(
      gpu_queue_submission, matching_gpu_job_option.value(), string_intern_pool,
      get_string_hash_and_send_to_listener_if_necessary);

  if (!HasUnprocessedBeginMarkers(thread_id, post_submission_cpu_timestamp)) {
    DeleteSavedGpuJob(thread_id, matching_gpu_job_option->amdgpu_cs_ioctl_time_ns());
  }
  return result;
}
std::vector<orbit_client_protos::TimerInfo> GpuQueueSubmissionProcessor::ProcessGpuJob(
    const orbit_grpc_protos::GpuJob& gpu_job,
    const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool,
    const std::function<uint64_t(const std::string& str)>&
        get_string_hash_and_send_to_listener_if_necessary) {
  int32_t thread_id = gpu_job.tid();
  uint64_t amdgpu_cs_ioctl_time_ns = gpu_job.amdgpu_cs_ioctl_time_ns();
  std::optional<GpuQueueSubmission> matching_gpu_submission =
      FindMatchingGpuQueueSubmission(thread_id, amdgpu_cs_ioctl_time_ns);

  // If we haven't found the matching "GpuSubmission" or the submission contains "begin" markers
  // (which might have the "end" markers in a later submission), we save the "GpuJob" for later.
  // Note that as soon as all "begin" markers have been processed, the "GpuJob" will be deleted
  // again.
  if (!matching_gpu_submission.has_value() || matching_gpu_submission->num_begin_markers() > 0) {
    tid_to_submission_time_to_gpu_job_[thread_id][amdgpu_cs_ioctl_time_ns] = gpu_job;
  }
  if (!matching_gpu_submission.has_value()) {
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

std::optional<GpuQueueSubmission> GpuQueueSubmissionProcessor::FindMatchingGpuQueueSubmission(
    int32_t thread_id, uint64_t submit_time) {
  const auto& post_submission_time_to_gpu_submission_it =
      tid_to_post_submission_time_to_gpu_submission_.find(thread_id);
  if (post_submission_time_to_gpu_submission_it ==
      tid_to_post_submission_time_to_gpu_submission_.end()) {
    return std::nullopt;
  }

  const auto& post_submission_time_to_gpu_submission =
      post_submission_time_to_gpu_submission_it->second;

  // Find the first Gpu submission with a "post submission" timestamp greater or equal to the Gpu
  // job's timestamp. If the "pre submission" timestamp is not greater (i.e. less or equal) than the
  // job's timestamp, we have found the matching submission.
  auto lower_bound_gpu_submission_it =
      post_submission_time_to_gpu_submission.lower_bound(submit_time);
  if (lower_bound_gpu_submission_it == post_submission_time_to_gpu_submission.end()) {
    return std::nullopt;
  }
  const GpuQueueSubmission& matching_gpu_submission = lower_bound_gpu_submission_it->second;

  if (matching_gpu_submission.meta_info().pre_submission_cpu_timestamp() > submit_time) {
    return std::nullopt;
  }

  return matching_gpu_submission;
}

std::optional<GpuJob> GpuQueueSubmissionProcessor::FindMatchingGpuJob(
    int32_t thread_id, uint64_t pre_submission_cpu_timestamp,
    uint64_t post_submission_cpu_timestamp) {
  const auto& submission_time_to_gpu_job_it = tid_to_submission_time_to_gpu_job_.find(thread_id);
  if (submission_time_to_gpu_job_it == tid_to_submission_time_to_gpu_job_.end()) {
    return std::nullopt;
  }

  const auto& submission_time_to_gpu_job = submission_time_to_gpu_job_it->second;

  // Find the first Gpu job that has a timestamp greater or equal to the "pre submission" timestamp:
  auto gpu_job_matching_pre_submission_it =
      submission_time_to_gpu_job.lower_bound(pre_submission_cpu_timestamp);
  if (gpu_job_matching_pre_submission_it == submission_time_to_gpu_job.end()) {
    return std::nullopt;
  }

  // Find the first Gpu job that has a timestamp greater to the "post submission" timestamp
  // (which would be the next job) and decrease the iterator by one.
  auto gpu_job_matching_post_submission_it =
      submission_time_to_gpu_job.upper_bound(post_submission_cpu_timestamp);
  if (gpu_job_matching_post_submission_it == submission_time_to_gpu_job.begin()) {
    return std::nullopt;
  }
  --gpu_job_matching_post_submission_it;

  if (&gpu_job_matching_pre_submission_it->second != &gpu_job_matching_post_submission_it->second) {
    return std::nullopt;
  }

  return gpu_job_matching_pre_submission_it->second;
}

std::vector<TimerInfo> GpuQueueSubmissionProcessor::ProcessGpuQueueSubmissionWithMatchingGpuJob(
    const GpuQueueSubmission& gpu_queue_submission, const GpuJob& matching_gpu_job,
    const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool,
    const std::function<uint64_t(const std::string& str)>&
        get_string_hash_and_send_to_listener_if_necessary) {
  std::vector<TimerInfo> result;
  std::string timeline;
  if (matching_gpu_job.timeline_or_key_case() == GpuJob::kTimelineKey) {
    CHECK(string_intern_pool.contains(matching_gpu_job.timeline_key()));
    timeline = string_intern_pool.at(matching_gpu_job.timeline_key());
  } else {
    timeline = matching_gpu_job.timeline();
  }
  uint64_t timeline_hash = get_string_hash_and_send_to_listener_if_necessary(timeline);

  std::optional<GpuCommandBuffer> first_command_buffer =
      ExtractFirstCommandBuffer(gpu_queue_submission);

  std::vector<TimerInfo> command_buffer_timers =
      ProcessGpuCommandBuffers(gpu_queue_submission, matching_gpu_job, first_command_buffer,
                               timeline_hash, get_string_hash_and_send_to_listener_if_necessary);

  result.insert(result.end(), command_buffer_timers.begin(), command_buffer_timers.end());

  std::vector<TimerInfo> debug_marker_timers =
      ProcessGpuDebugMarkers(gpu_queue_submission, matching_gpu_job, first_command_buffer, timeline,
                             string_intern_pool, get_string_hash_and_send_to_listener_if_necessary);

  result.insert(result.end(), debug_marker_timers.begin(), debug_marker_timers.end());

  return result;
}

bool GpuQueueSubmissionProcessor::HasUnprocessedBeginMarkers(
    int32_t thread_id, uint64_t post_submission_timestamp) const {
  if (!tid_to_post_submission_time_to_num_begin_markers_.contains(thread_id)) {
    return false;
  }
  if (!tid_to_post_submission_time_to_num_begin_markers_.at(thread_id).contains(
          post_submission_timestamp)) {
    return false;
  }
  CHECK(tid_to_post_submission_time_to_num_begin_markers_.at(thread_id).at(
            post_submission_timestamp) > 0);
  return true;
}

void GpuQueueSubmissionProcessor::DecrementUnprocessedBeginMarkers(
    int32_t thread_id, uint64_t submission_timestamp, uint64_t post_submission_timestamp) {
  CHECK(tid_to_post_submission_time_to_num_begin_markers_.contains(thread_id));
  auto& post_submission_time_to_num_begin_markers =
      tid_to_post_submission_time_to_num_begin_markers_.at(thread_id);
  CHECK(post_submission_time_to_num_begin_markers.contains(post_submission_timestamp));
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

void GpuQueueSubmissionProcessor::DeleteSavedGpuJob(int32_t thread_id,
                                                    uint64_t submission_timestamp) {
  if (!tid_to_submission_time_to_gpu_job_.contains(thread_id)) {
    return;
  }
  auto& submission_time_to_gpu_job = tid_to_submission_time_to_gpu_job_.at(thread_id);
  submission_time_to_gpu_job.erase(submission_timestamp);
  if (submission_time_to_gpu_job.empty()) {
    tid_to_submission_time_to_gpu_job_.erase(thread_id);
  }
}
void GpuQueueSubmissionProcessor::DeleteSavedGpuSubmission(int32_t thread_id,
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
    const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission,
    const orbit_grpc_protos::GpuJob& matching_gpu_job,
    const std::optional<orbit_grpc_protos::GpuCommandBuffer>& first_command_buffer,
    uint64_t timeline_hash,
    const std::function<uint64_t(const std::string& str)>&
        get_string_hash_and_send_to_listener_if_necessary) {
  constexpr const char* kCommandBufferLabel = "command buffer";
  uint64_t command_buffer_text_key =
      get_string_hash_and_send_to_listener_if_necessary(kCommandBufferLabel);

  int32_t thread_id = gpu_queue_submission.meta_info().tid();

  std::vector<TimerInfo> result;

  for (const auto& submit_info : gpu_queue_submission.submit_infos()) {
    for (const auto& command_buffer : submit_info.command_buffers()) {
      CHECK(first_command_buffer != std::nullopt);
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
      command_buffer_timer.set_type(TimerInfo::kGpuCommandBuffer);
      command_buffer_timer.set_user_data_key(command_buffer_text_key);
      result.push_back(command_buffer_timer);
    }
  }
  return result;
}

std::vector<TimerInfo> GpuQueueSubmissionProcessor::ProcessGpuDebugMarkers(
    const GpuQueueSubmission& gpu_queue_submission, const GpuJob& matching_gpu_job,
    const std::optional<GpuCommandBuffer>& first_command_buffer, const std::string& timeline,
    const absl::flat_hash_map<uint64_t, std::string>& string_intern_pool,
    const std::function<uint64_t(const std::string& str)>&
        get_string_hash_and_send_to_listener_if_necessary) {
  if (gpu_queue_submission.completed_markers_size() == 0) {
    return {};
  }
  std::vector<TimerInfo> result;
  std::string timeline_marker = timeline + "_marker";
  uint64_t timeline_marker_hash =
      get_string_hash_and_send_to_listener_if_necessary(timeline_marker);

  const auto& submission_meta_info = gpu_queue_submission.meta_info();
  const int32_t submission_thread_id = submission_meta_info.tid();
  uint64_t submission_pre_submission_cpu_timestamp =
      submission_meta_info.pre_submission_cpu_timestamp();
  uint64_t submission_post_submission_cpu_timestamp =
      submission_meta_info.post_submission_cpu_timestamp();

  static constexpr int32_t kUnknownThreadId = -1;

  for (const auto& completed_marker : gpu_queue_submission.completed_markers()) {
    CHECK(first_command_buffer != std::nullopt);
    TimerInfo marker_timer;

    // If we've recorded the submission that contains the begin marker, we'll retrieve this
    // submission from our mappings, and set the markers begin time accordingly.
    // Otherwise, we will use the capture start time as begin.
    if (completed_marker.has_begin_marker()) {
      const auto& begin_marker_info = completed_marker.begin_marker();
      const auto& begin_marker_meta_info = begin_marker_info.meta_info();
      const int32_t begin_marker_thread_id = begin_marker_meta_info.tid();
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
        std::optional<GpuQueueSubmission> matching_begin_submission =
            FindMatchingGpuQueueSubmission(begin_marker_thread_id,
                                           begin_marker_post_submission_cpu_timestamp);
        // Note that we receive submissions of a single queue in order (by CPU submission time). So
        // if there is no matching "begin submission", the "begin" was submitted before the "end"
        // and we lost the record of the "begin submission" (which should not happen).
        CHECK(matching_begin_submission.has_value());
        begin_submission_first_command_buffer =
            ExtractFirstCommandBuffer(matching_begin_submission.value());
      }
      CHECK(begin_submission_first_command_buffer.has_value());

      std::optional<GpuJob> matching_begin_job = FindMatchingGpuJob(
          begin_marker_thread_id, begin_marker_meta_info.pre_submission_cpu_timestamp(),
          begin_marker_post_submission_cpu_timestamp);
      CHECK(matching_begin_job.has_value());

      // Convert the GPU time to CPU time, based on the CPU time of the HW execution begin and the
      // GPU timestamp of the begin of the first command buffer. Note that we will assume that the
      // first command buffer starts execution right away as an approximation.
      marker_timer.set_start(completed_marker.begin_marker().gpu_timestamp_ns() +
                             matching_begin_job->gpu_hardware_start_time_ns() -
                             begin_submission_first_command_buffer->begin_gpu_timestamp_ns());
      if (begin_marker_thread_id == gpu_queue_submission.meta_info().tid()) {
        marker_timer.set_thread_id(begin_marker_thread_id);
      } else {
        marker_timer.set_thread_id(kUnknownThreadId);
      }

      DecrementUnprocessedBeginMarkers(begin_marker_thread_id,
                                       matching_begin_job->amdgpu_cs_ioctl_time_ns(),
                                       begin_marker_post_submission_cpu_timestamp);
    } else {
      marker_timer.set_start(begin_capture_time_ns_);
      marker_timer.set_thread_id(kUnknownThreadId);
    }

    marker_timer.set_depth(completed_marker.depth());
    marker_timer.set_timeline_hash(timeline_marker_hash);
    marker_timer.set_processor(-1);
    marker_timer.set_type(TimerInfo::kGpuDebugMarker);
    marker_timer.set_end(completed_marker.end_gpu_timestamp_ns() -
                         first_command_buffer->begin_gpu_timestamp_ns() +
                         matching_gpu_job.gpu_hardware_start_time_ns());

    CHECK(string_intern_pool.contains(completed_marker.text_key()));
    const std::string& text = string_intern_pool.at(completed_marker.text_key());
    uint64_t text_key = get_string_hash_and_send_to_listener_if_necessary(text);

    if (completed_marker.has_color()) {
      Color* color = marker_timer.mutable_color();
      color->set_red(static_cast<uint32_t>(completed_marker.color().red() * 255.f));
      color->set_green(static_cast<uint32_t>(completed_marker.color().green() * 255.f));
      color->set_blue(static_cast<uint32_t>(completed_marker.color().blue() * 255.f));
      color->set_alpha(static_cast<uint32_t>(completed_marker.color().alpha() * 255.f));
    }
    marker_timer.set_user_data_key(text_key);
    result.push_back(marker_timer);
  }
  return result;
}

std::optional<orbit_grpc_protos::GpuCommandBuffer>
GpuQueueSubmissionProcessor::ExtractFirstCommandBuffer(
    const orbit_grpc_protos::GpuQueueSubmission& gpu_queue_submission) {
  for (const auto& submit_info : gpu_queue_submission.submit_infos()) {
    for (const auto& command_buffer : submit_info.command_buffers()) {
      return command_buffer;
    }
  }
  return std::nullopt;
}
