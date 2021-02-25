// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_SUBMISSION_TRACKER_H_
#define ORBIT_VULKAN_LAYER_SUBMISSION_TRACKER_H_

#include <stack>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"
#include "VulkanLayerProducer.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "vulkan/vulkan.h"

namespace orbit_vulkan_layer {

/*
 * This class is responsible for tracking command buffer and debug marker timings.
 * To do so, it keeps track of command-buffer allocations, destructions, begins, ends as well as
 * submissions.
 * If we are capturing on `VkBeginCommandBuffer` and `VkEndCommandBuffer` it will insert write
 * timestamp commands (`VkCmdWriteTimestamp`). The same is done for debug marker begins and ends.
 * All that data will be gathered together at a queue submission (`VkQueueSubmit`).
 *
 * Upon every `VkQueuePresentKHR` it will check if the last timestamp of a certain submission is
 * already available, and if so, it will assume that all timestamps are available and it will send
 * the results over to the `VulkanLayerProducer`.
 *
 * See also `DispatchTable` (for vulkan dispatch), `TimerQueryPool` (to manage the timestamp slots),
 * and `DeviceManager` (to retrieve device properties).
 *
 * Thread-Safety: This class is internally synchronized (using read/write locks), and can be
 * safely accessed from different threads. This is needed, as in Vulkan submits and command buffer
 * modifications can happen from multiple threads.
 */
template <class DispatchTable, class DeviceManager, class TimerQueryPool>
class SubmissionTracker : public VulkanLayerProducer::CaptureStatusListener {
 public:
  // On a submission (vkQueueSubmit), all pointers to command buffers become invalid/can be reused
  // for a the next submission. The struct `QueueSubmission` gathers information (like timestamps
  // and timer slots) about a concrete submission and their corresponding command buffers and debug
  // markers. This way the information is "persistent" across submissions. We will create this
  // struct at `vkQueuePresent` (right before the call into the driver -- and only if we are
  // capturing) and add some information (in particular markers and a timestamp) right after the
  // driver call. So, we will use `QueueSubmission` as a return value, and therefore these structs
  // need to be public in the `SubmissionTracker`.

  // Stores meta information about a submit (VkQueueSubmit), e.g. to allow to identify the matching
  // Gpu tracepoint.
  struct SubmissionMetaInformation {
    uint64_t pre_submission_cpu_timestamp;
    uint64_t post_submission_cpu_timestamp;
    int32_t thread_id;
  };

  // A persistent version of a command buffer that was submitted and its begin/end slot in the
  // `TimerQueryPool`. Note that the begin is optional, as it might not be part of the capture.
  // This struct is created if we capture the submission, so if we haven't captured the end,
  // we also haven't captured the begin, and don't need to store info about that command buffer at
  // all. The list of all `SubmittedCommandBuffer`s is stored in a `QueueSubmission`, and can so be
  // associated to the `SubmissionMetaInformation`.
  struct SubmittedCommandBuffer {
    std::optional<uint32_t> command_buffer_begin_slot_index;
    uint32_t command_buffer_end_slot_index;

    // These are set when the timestamps were queried successfully. When these have values, the
    // corresponding slot indices for timestamp queries have been reset. Do not query again in
    // that case and only get the timestamps through these members.
    std::optional<uint64_t> begin_timestamp;
    std::optional<uint64_t> end_timestamp;
  };

  // A persistent version of a debug marker (either begin or end) that was submitted and its slot
  // in the `TimerQueryPool`. `SubmittedMarkers` are used in `MarkerState` to identify the "begin"
  // or "end" marker. All markers (`MarkerState`s) that gets *completed* within a certain submission
  // are stored in the `QueueSubmission`.
  // Note that we also store the `SubmissionMetaInformation` as "begin" markers can originate from
  // different submissions than the matching end markers. This allows us e.g. to match the markers
  // to a specific submission tracepoint.
  struct SubmittedMarker {
    SubmissionMetaInformation meta_information;
    uint32_t slot_index;

    // This is set when the timestamp was queried successfully. When this has a value, the
    // corresponding slot index for the timestamp query has been reset. Do not query again in
    // that case and only get the timestamp through this member.
    std::optional<uint64_t> timestamp;
  };

  // Represents a color to be used to in debug markers. The values are all in range [0.f, 1.f].
  struct Color {
    float red;
    float green;
    float blue;
    float alpha;
  };

  // Identifies a particular debug marker region that has been submitted via `vkQueueSubmit`.
  // Note that we only store the state into `QueueSubmission`, if at that time we have a value for
  // the end_info. Beside the information about the begin/end, it also stores the label name, color
  // and the depth of the marker.
  struct SubmittedMarkerSlice {
    std::optional<SubmittedMarker> begin_info;
    SubmittedMarker end_info;
    std::string label_name;
    Color color;
    size_t depth;
  };

  // A single submission (VkQueueSubmit) can contain multiple `SubmitInfo`s. We keep this structure.
  struct SubmitInfo {
    std::vector<SubmittedCommandBuffer> command_buffers;
  };

  // Wraps up all the data that needs to be persistent upon a submission (VkQueueSubmit).
  // `completed_markers` are all the debug markers that got completed (via "End") within this
  // submission. Their "Begin" might still be in a different submission.
  struct QueueSubmission {
    VkQueue queue;
    SubmissionMetaInformation meta_information;
    std::vector<SubmitInfo> submit_infos;
    std::vector<SubmittedMarkerSlice> completed_markers;
    uint32_t num_begin_markers = 0;
  };

  explicit SubmissionTracker(DispatchTable* dispatch_table, TimerQueryPool* timer_query_pool,
                             DeviceManager* device_manager,
                             uint32_t max_local_marker_depth_per_command_buffer)
      : dispatch_table_(dispatch_table),
        timer_query_pool_(timer_query_pool),
        device_manager_(device_manager),
        max_local_marker_depth_per_command_buffer_(max_local_marker_depth_per_command_buffer) {
    CHECK(dispatch_table_ != nullptr);
    CHECK(timer_query_pool_ != nullptr);
    CHECK(device_manager_ != nullptr);
  }

  // Sets a producer to be used to enqueue capture events and asks if we are capturing.
  // We will also register ourselves as CaptureStatusListener, in order to get notified on
  // capture finish (OnCaptureFinished). That's when we will reset the open query slots.
  void SetVulkanLayerProducer(VulkanLayerProducer* vulkan_layer_producer) {
    vulkan_layer_producer_ = vulkan_layer_producer;
    if (vulkan_layer_producer_ != nullptr) {
      vulkan_layer_producer_->SetCaptureStatusListener(this);
    } else {
      vulkan_layer_producer->SetCaptureStatusListener(nullptr);
    }
  }

  // Sets the maximum depth of debug markers within one command buffer. If set to 0, all debug
  // markers will be discarded. If set to to std::numeric_limits<uint32_t>::max(), no debug marker
  // will be discarded.
  void SetMaxLocalMarkerDepthPerCommandBuffer(uint32_t max_local_marker_depth_per_command_buffer) {
    max_local_marker_depth_per_command_buffer_ = max_local_marker_depth_per_command_buffer;
  }

  void TrackCommandBuffers(VkDevice device, VkCommandPool pool,
                           const VkCommandBuffer* command_buffers, uint32_t count) {
    absl::WriterMutexLock lock(&mutex_);
    auto associated_cbs_it = pool_to_command_buffers_.find(pool);
    if (associated_cbs_it == pool_to_command_buffers_.end()) {
      associated_cbs_it = pool_to_command_buffers_.try_emplace(pool).first;
    }
    for (uint32_t i = 0; i < count; ++i) {
      VkCommandBuffer cb = command_buffers[i];
      associated_cbs_it->second.insert(cb);
      command_buffer_to_device_[cb] = device;
    }
  }

  void UntrackCommandBuffers(VkDevice device, VkCommandPool pool,
                             const VkCommandBuffer* command_buffers, uint32_t count) {
    absl::WriterMutexLock lock(&mutex_);
    CHECK(pool_to_command_buffers_.contains(pool));
    absl::flat_hash_set<VkCommandBuffer>& associated_command_buffers =
        pool_to_command_buffers_.at(pool);
    for (uint32_t i = 0; i < count; ++i) {
      VkCommandBuffer command_buffer = command_buffers[i];
      associated_command_buffers.erase(command_buffer);

      // vkFreeCommandBuffers (and thus this method) can be also called on command bufers in
      // "recording" or executable state and has similar effect as vkResetCommandBuffer has.
      // In `OnCaptureFinished`, we reset all the timer slots left in `command_buffer_to_state_`.
      // If we would not reset them here and clear the state, we would try to reset those command
      // buffers there. However, the mapping to the device (which is needed) would be missing.
      if (command_buffer_to_state_.contains(command_buffer)) {
        ResetCommandBufferUnsafe(command_buffer);

        command_buffer_to_state_.erase(command_buffer);
      }

      CHECK(command_buffer_to_device_.contains(command_buffer));
      CHECK(command_buffer_to_device_.at(command_buffer) == device);
      command_buffer_to_device_.erase(command_buffer);
    }
    if (associated_command_buffers.empty()) {
      pool_to_command_buffers_.erase(pool);
    }
  }

  void MarkCommandBufferBegin(VkCommandBuffer command_buffer) {
    // Even when we are not capturing we create state for this command buffer to allow the
    // debug marker tracking. In order to compute the correct depth of a debug marker and being able
    // to match an "end" marker with the corresponding "begin" marker, we maintain a stack of all
    // debug markers of a queue. The order of the markers is determined upon submission based on the
    // order within the submitted command buffers. Thus, even when not capturing, we create an empty
    // state here that allows us to store the debug markers into it and maintain that stack on
    // submission. We will not write timestamps in this case and thus don't store any information
    // other than the debug markers then.
    {
      absl::WriterMutexLock lock(&mutex_);
      CHECK(!command_buffer_to_state_.contains(command_buffer));
      command_buffer_to_state_[command_buffer] = {};
    }
    if (!IsCapturing()) {
      return;
    }

    uint32_t slot_index = RecordTimestamp(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    {
      absl::WriterMutexLock lock(&mutex_);
      CHECK(command_buffer_to_state_.contains(command_buffer));
      command_buffer_to_state_.at(command_buffer).command_buffer_begin_slot_index =
          std::make_optional(slot_index);
    }
  }

  void MarkCommandBufferEnd(VkCommandBuffer command_buffer) {
    if (!IsCapturing()) {
      return;
    }

    uint32_t slot_index = RecordTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

    {
      absl::ReaderMutexLock lock(&mutex_);
      // MarkCommandBufferBegin/End are called from within the same submit, and as the
      // `MarkCommandBufferBegin` will always insert the state, we can assume that it is there.
      CHECK(command_buffer_to_state_.contains(command_buffer));
      CommandBufferState& command_buffer_state = command_buffer_to_state_.at(command_buffer);
      // Writing to this field is safe, as there can't be any operation on this command buffer
      // in parallel.
      command_buffer_state.command_buffer_end_slot_index = std::make_optional(slot_index);
    }
  }

  void MarkDebugMarkerBegin(VkCommandBuffer command_buffer, const char* text, Color color) {
    // It is ensured by the Vulkan spec. that `text` must not be nullptr.
    CHECK(text != nullptr);
    bool marker_depth_exceeds_maximum;
    {
      absl::WriterMutexLock lock(&mutex_);
      CHECK(command_buffer_to_state_.contains(command_buffer));
      CommandBufferState& state = command_buffer_to_state_.at(command_buffer);
      ++state.local_marker_stack_size;
      marker_depth_exceeds_maximum =
          state.local_marker_stack_size > max_local_marker_depth_per_command_buffer_;
      Marker marker{.type = MarkerType::kDebugMarkerBegin,
                    .label_name = std::string(text),
                    .color = color,
                    .cut_off = marker_depth_exceeds_maximum};
      state.markers.emplace_back(std::move(marker));
    }

    if (!IsCapturing() || marker_depth_exceeds_maximum) {
      return;
    }

    uint32_t slot_index = RecordTimestamp(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    {
      absl::WriterMutexLock lock(&mutex_);
      CHECK(command_buffer_to_state_.contains(command_buffer));
      CommandBufferState& state = command_buffer_to_state_.at(command_buffer);
      state.markers.back().slot_index = std::make_optional(slot_index);
    }
  }

  void MarkDebugMarkerEnd(VkCommandBuffer command_buffer) {
    bool marker_depth_exceeds_maximum;
    {
      absl::WriterMutexLock lock(&mutex_);
      CHECK(command_buffer_to_state_.contains(command_buffer));
      CommandBufferState& state = command_buffer_to_state_.at(command_buffer);
      marker_depth_exceeds_maximum =
          state.local_marker_stack_size > max_local_marker_depth_per_command_buffer_;
      Marker marker{.type = MarkerType::kDebugMarkerEnd, .cut_off = marker_depth_exceeds_maximum};
      state.markers.emplace_back(std::move(marker));
      // We might see more "ends" than "begins", as the "begins" can be on a different command
      // buffer.
      if (state.local_marker_stack_size != 0) {
        --state.local_marker_stack_size;
      }
    }

    if (!IsCapturing() || marker_depth_exceeds_maximum) {
      return;
    }

    uint32_t slot_index = RecordTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    {
      absl::WriterMutexLock lock(&mutex_);
      CHECK(command_buffer_to_state_.contains(command_buffer));
      CommandBufferState& state = command_buffer_to_state_.at(command_buffer);
      state.markers.back().slot_index = std::make_optional(slot_index);
    }
  }

  // After command buffers are submitted into a queue, they can be reused for further operations.
  // Thus, our identification via the pointers becomes invalid. We will use the vkQueueSubmit
  // to make our data persistent until we have processed the results of the execution of these
  // command buffers (which will be done in the vkQueuePresentKHR).
  // If we are not capturing, that method will do nothing and return `nullopt`.
  // Otherwise, it will create and return an QueueSubmission object, holding the
  // information about all command buffers recorded in this submission.
  // It also takes a timestamp before the execution of the driver code for the submission.
  // This allows us to map submissions from the Vulkan layer to the driver submissions.
  [[nodiscard]] std::optional<QueueSubmission> PersistCommandBuffersOnSubmit(
      VkQueue queue, uint32_t submit_count, const VkSubmitInfo* submits) {
    if (!IsCapturing()) {
      // `PersistDebugMarkersOnSubmit` and `OnCaptureFinished` will take care of clean up and
      // slot resetting.
      return std::nullopt;
    }

    QueueSubmission queue_submission;
    queue_submission.queue = queue;
    queue_submission.meta_information.pre_submission_cpu_timestamp =
        orbit_base::CaptureTimestampNs();
    queue_submission.meta_information.thread_id = orbit_base::GetCurrentThreadId();

    for (uint32_t submit_index = 0; submit_index < submit_count; ++submit_index) {
      VkSubmitInfo submit_info = submits[submit_index];
      queue_submission.submit_infos.emplace_back();
      SubmitInfo& submitted_submit_info = queue_submission.submit_infos.back();
      for (uint32_t command_buffer_index = 0; command_buffer_index < submit_info.commandBufferCount;
           ++command_buffer_index) {
        VkCommandBuffer command_buffer = submit_info.pCommandBuffers[command_buffer_index];
        CHECK(command_buffer_to_state_.contains(command_buffer));
        CommandBufferState& state = command_buffer_to_state_.at(command_buffer);

        // If we haven't recorded neither the end nor the begin of a command buffer, we have no
        // information to send.
        if (!state.command_buffer_end_slot_index.has_value()) {
          continue;
        }

        SubmittedCommandBuffer submitted_command_buffer{
            .command_buffer_begin_slot_index = state.command_buffer_begin_slot_index,
            .command_buffer_end_slot_index = state.command_buffer_end_slot_index.value()};
        submitted_submit_info.command_buffers.emplace_back(submitted_command_buffer);

        // Remove the slots from the state now such that `OnCaptureFinished` won't reset the slots
        // twice. Note that they will be reset in `CompleteSubmits`.
        state.command_buffer_begin_slot_index.reset();
        state.command_buffer_end_slot_index.reset();
      }
    }

    return queue_submission;
  }

  // This method is supposed to be called right after the driver call of `vkQueuePresent`.
  // At that point in time we can complete the submission meta information (add the timestamp) and
  // know the order of the debug markers across command buffers. We will maintain the debug marker
  // stack in `queue_to_markers_` and if capturing also write the information about completed debug
  // markers into the `QueueSubmission`, to make it persistent across submissions and such that it
  // can be picked up later (on a vkQueuePresentKHR) to retrieve the timer results and send the data
  // to the client.
  //
  // We assume to be capturing if `queue_submission_optional` has content, i.e. we were capturing
  // on this VkQueueSubmit before calling into the driver.
  // The meta information allows us to map submissions from the Vulkan layer to the driver
  // submissions.
  void PersistDebugMarkersOnSubmit(VkQueue queue, uint32_t submit_count,
                                   const VkSubmitInfo* submits,
                                   std::optional<QueueSubmission> queue_submission_optional) {
    absl::WriterMutexLock lock(&mutex_);
    if (!queue_to_markers_.contains(queue)) {
      queue_to_markers_[queue] = {};
    }
    CHECK(queue_to_markers_.contains(queue));
    QueueMarkerState& markers = queue_to_markers_.at(queue);

    // If we consider that we are still capturing, take a cpu timestamp as "post submission" such
    // that the submission "meta information" is complete. We can then attach that also to each
    // debug marker, as they may span across different submissions.
    if (queue_submission_optional.has_value()) {
      queue_submission_optional->meta_information.post_submission_cpu_timestamp =
          orbit_base::CaptureTimestampNs();
    }

    std::vector<uint32_t> marker_slots_to_reset;
    VkDevice device = VK_NULL_HANDLE;
    for (uint32_t submit_index = 0; submit_index < submit_count; ++submit_index) {
      VkSubmitInfo submit_info = submits[submit_index];
      for (uint32_t command_buffer_index = 0; command_buffer_index < submit_info.commandBufferCount;
           ++command_buffer_index) {
        VkCommandBuffer command_buffer = submit_info.pCommandBuffers[command_buffer_index];
        if (device == VK_NULL_HANDLE) {
          CHECK(command_buffer_to_device_.contains(command_buffer));
          device = command_buffer_to_device_.at(command_buffer);
        }
        CHECK(command_buffer_to_state_.contains(command_buffer));
        CommandBufferState& state = command_buffer_to_state_.at(command_buffer);

        for (const Marker& marker : state.markers) {
          std::optional<SubmittedMarker> submitted_marker = std::nullopt;
          if (marker.slot_index.has_value() && queue_submission_optional.has_value()) {
            submitted_marker = {.meta_information = queue_submission_optional->meta_information,
                                .slot_index = marker.slot_index.value()};
          }

          switch (marker.type) {
            case MarkerType::kDebugMarkerBegin: {
              if (queue_submission_optional.has_value() && marker.slot_index.has_value()) {
                ++queue_submission_optional->num_begin_markers;
              }
              CHECK(marker.label_name.has_value());
              CHECK(marker.color.has_value());
              MarkerState marker_state{.label_name = marker.label_name.value(),
                                       .color = marker.color.value(),
                                       .begin_info = submitted_marker,
                                       .depth = markers.marker_stack.size(),
                                       .depth_exceeds_maximum = marker.cut_off};
              markers.marker_stack.push(std::move(marker_state));
              break;
            }

            case MarkerType::kDebugMarkerEnd: {
              MarkerState marker_state = markers.marker_stack.top();
              markers.marker_stack.pop();

              // If there is a begin marker slot from a previous submission, this is our chance to
              // reset the slot.
              if (marker_state.begin_info.has_value() && !queue_submission_optional.has_value()) {
                marker_slots_to_reset.push_back(marker_state.begin_info->slot_index);
              }

              // If the begin marker was discarded for exceeding the maximum depth, but the end
              // marker was not (as it is in a different submission), we reset the slot.
              if (marker_state.depth_exceeds_maximum && marker.slot_index.has_value()) {
                marker_slots_to_reset.push_back(marker.slot_index.value());
              }

              if (queue_submission_optional.has_value() && marker.slot_index.has_value() &&
                  !marker_state.depth_exceeds_maximum) {
                CHECK(submitted_marker.has_value());
                queue_submission_optional->completed_markers.emplace_back(
                    SubmittedMarkerSlice{.begin_info = marker_state.begin_info,
                                         .end_info = submitted_marker.value(),
                                         .depth = marker_state.depth,
                                         .color = marker_state.color,
                                         .label_name = std::move(marker_state.label_name)});
              }

              break;
            }
          }
        }
        command_buffer_to_state_.erase(command_buffer);
      }
    }

    if (!marker_slots_to_reset.empty()) {
      timer_query_pool_->ResetQuerySlots(device, marker_slots_to_reset);
    }

    if (!queue_submission_optional.has_value()) {
      return;
    }

    if (!queue_to_submissions_.contains(queue)) {
      queue_to_submissions_[queue] = {};
    }
    CHECK(queue_to_submissions_.contains(queue));
    queue_to_submissions_.at(queue).emplace_back(std::move(queue_submission_optional.value()));
  }

  // This method is responsible for retrieving all the timestamps for the "completed" submissions,
  // for transforming the information of those submissions (in particlular about the command buffers
  // and debug markers) into the `GpuQueueSubmission` proto, and for sending it to the
  // `VulkanLayerProducer`. We consider a submission to be "completed" if its last command buffer
  // timestamp is ready (see `PullCompletedSubmissions`).
  // Beside the timestamps of command buffers and the meta information of the submission, the proto
  // also contains the debug markers, "begin" (even if submitted in a different submission) and
  // "end", that got completed in this submission.
  // See also `WriteMetaInfo`, `WriteCommandBufferTimings` and `WriteDebugMarkers`.
  // This method also resets all the timer slots that have been read.
  // It is assumed to be called periodically, e.g. on `vkQueuePresentKHR`.
  void CompleteSubmits(VkDevice device) {
    VkQueryPool query_pool = timer_query_pool_->GetQueryPool(device);
    std::vector<QueueSubmission> completed_submissions =
        PullCompletedSubmissions(device, query_pool);

    if (completed_submissions.empty()) {
      return;
    }

    VkPhysicalDevice physical_device = device_manager_->GetPhysicalDeviceOfLogicalDevice(device);
    const float timestamp_period =
        device_manager_->GetPhysicalDeviceProperties(physical_device).limits.timestampPeriod;

    std::vector<uint32_t> query_slots_to_reset = {};
    std::vector<QueueSubmission> submissions_to_send = {};

    // The submits in completed_submissions are sorted by "pre submission CPU" timestamp and we want
    // to make sure we send events to the client in that order. Therefore, we must retry all
    // submissions after we encounter the first submission that we can't complete yet and push them
    // back into the queue_to_submissions_ queue.
    bool all_succeeded_so_far = true;
    for (auto& completed_submission : completed_submissions) {
      bool command_buffer_queries_succeeded = QueryCommandBufferTimestamps(
          &completed_submission, &query_slots_to_reset, device, query_pool, timestamp_period);
      bool marker_queries_succeeded = QueryDebugMarkerTimestamps(
          &completed_submission, &query_slots_to_reset, device, query_pool, timestamp_period);

      all_succeeded_so_far &= command_buffer_queries_succeeded && marker_queries_succeeded;

      if (all_succeeded_so_far) {
        submissions_to_send.push_back(completed_submission);
      } else {
        VkQueue queue = completed_submission.queue;

        if (!queue_to_submissions_.contains(queue)) {
          queue_to_submissions_[queue] = {};
        }
        CHECK(queue_to_submissions_.contains(queue));
        queue_to_submissions_.at(queue).emplace_back(std::move(completed_submission));
      }
    }

    for (const auto& completed_submission : submissions_to_send) {
      orbit_grpc_protos::ProducerCaptureEvent capture_event;
      orbit_grpc_protos::GpuQueueSubmission* submission_proto =
          capture_event.mutable_gpu_queue_submission();

      WriteMetaInfo(completed_submission.meta_information, submission_proto->mutable_meta_info());
      WriteCommandBufferTimings(completed_submission, submission_proto);
      WriteDebugMarkers(completed_submission, submission_proto);

      if (vulkan_layer_producer_ != nullptr) {
        vulkan_layer_producer_->EnqueueCaptureEvent(std::move(capture_event));
      }
    }

    timer_query_pool_->ResetQuerySlots(device, query_slots_to_reset);
  }

  void ResetCommandBuffer(VkCommandBuffer command_buffer) {
    absl::WriterMutexLock lock(&mutex_);
    ResetCommandBufferUnsafe(command_buffer);
  }

  void ResetCommandPool(VkCommandPool command_pool) {
    absl::flat_hash_set<VkCommandBuffer> command_buffers;
    {
      absl::ReaderMutexLock lock(&mutex_);
      if (!pool_to_command_buffers_.contains(command_pool)) {
        return;
      }
      CHECK(pool_to_command_buffers_.contains(command_pool));
      command_buffers = pool_to_command_buffers_.at(command_pool);
    }
    for (const auto& command_buffer : command_buffers) {
      ResetCommandBuffer(command_buffer);
    }
  }

  void OnCaptureStart(orbit_grpc_protos::CaptureOptions capture_options) override {
    SetMaxLocalMarkerDepthPerCommandBuffer(
        capture_options.max_local_marker_depth_per_command_buffer());
  }

  void OnCaptureStop() override {}

  void OnCaptureFinished() override {
    absl::WriterMutexLock lock(&mutex_);
    std::vector<uint32_t> slots_to_reset;

    VkDevice device = VK_NULL_HANDLE;

    for (auto& [command_buffer, command_buffer_state] : command_buffer_to_state_) {
      if (device == VK_NULL_HANDLE) {
        CHECK(command_buffer_to_device_.contains(command_buffer));
        device = command_buffer_to_device_.at(command_buffer);
      }
      if (command_buffer_state.command_buffer_begin_slot_index.has_value()) {
        slots_to_reset.push_back(command_buffer_state.command_buffer_begin_slot_index.value());
        command_buffer_state.command_buffer_begin_slot_index.reset();
      }

      if (command_buffer_state.command_buffer_end_slot_index.has_value()) {
        slots_to_reset.push_back(command_buffer_state.command_buffer_end_slot_index.value());
        command_buffer_state.command_buffer_end_slot_index.reset();
      }

      for (Marker& marker : command_buffer_state.markers) {
        if (marker.slot_index.has_value()) {
          slots_to_reset.push_back(marker.slot_index.value());
          marker.slot_index.reset();
        }
      }
    }
    if (!slots_to_reset.empty()) {
      timer_query_pool_->ResetQuerySlots(device, slots_to_reset);
    }
  }

 private:
  enum class MarkerType { kDebugMarkerBegin = 0, kDebugMarkerEnd };

  struct Marker {
    MarkerType type;
    std::optional<uint32_t> slot_index;
    std::optional<std::string> label_name;
    std::optional<Color> color;
    bool cut_off;
  };

  // We have a stack of all markers of a queue that gets updated upon a submission (VkQueueSubmit).
  // So on a submitted "begin" marker, we create that struct and push it to the stack. On an "end",
  // we pop from that stack, and if we are capturing we complete the information and move it to the
  // completed markers of a `QueueSubmission`.
  // If a debug marker begin was discarded because of its depth, `depth_exceeds_maximum` is set to
  // true. This allows end markers on a different submission to also throw the end marker away.
  //
  // Example: Max Depth = 1
  // Submission 1: Begin("Foo"), Begin("Bar) -- For "Bar" we set `depth_exceeds_maximum` to true.
  // Submission 2: End("Bar"), End("Foo") -- We now know that the first end needs to be thrown away.
  struct MarkerState {
    std::optional<SubmittedMarker> begin_info;
    std::string label_name;
    Color color;
    size_t depth;
    bool depth_exceeds_maximum;
  };

  struct QueueMarkerState {
    std::stack<MarkerState> marker_stack;
  };

  struct CommandBufferState {
    std::optional<uint32_t> command_buffer_begin_slot_index;
    std::optional<uint32_t> command_buffer_end_slot_index;
    std::vector<Marker> markers;
    uint32_t local_marker_stack_size;
  };

  uint32_t RecordTimestamp(VkCommandBuffer command_buffer,
                           VkPipelineStageFlagBits pipeline_stage_flags) {
    VkDevice device;
    {
      absl::ReaderMutexLock lock(&mutex_);
      CHECK(command_buffer_to_device_.contains(command_buffer));
      device = command_buffer_to_device_.at(command_buffer);
    }

    VkQueryPool query_pool = timer_query_pool_->GetQueryPool(device);

    uint32_t slot_index;
    bool found_slot = timer_query_pool_->NextReadyQuerySlot(device, &slot_index);
    CHECK(found_slot);
    dispatch_table_->CmdWriteTimestamp(command_buffer)(command_buffer, pipeline_stage_flags,
                                                       query_pool, slot_index);

    return slot_index;
  }

  std::vector<QueueSubmission> PullCompletedSubmissions(VkDevice device, VkQueryPool query_pool) {
    std::vector<QueueSubmission> completed_submissions = {};

    absl::WriterMutexLock lock(&mutex_);
    for (auto& [unused_queue, queue_submissions] : queue_to_submissions_) {
      auto submission_it = queue_submissions.begin();
      while (submission_it != queue_submissions.end()) {
        const QueueSubmission& submission = *submission_it;
        if (submission.submit_infos.empty()) {
          submission_it = queue_submissions.erase(submission_it);
          continue;
        }

        bool erase_submission = true;
        // Let's find the last command buffer in this submission, so first find the last
        // submit info that has at least one command buffer.
        // We test if for this command buffer we already have a query result for its last slot
        // and if so (or if the submission does not contain any command buffer) erase this
        // submission.
        // We expect that each `QueueSubmission` that made it into the `queue_to_submissions_`
        // contains at least one command buffer.
        auto submit_info_reverse_it = submission.submit_infos.rbegin();
        while (submit_info_reverse_it != submission.submit_infos.rend()) {
          const SubmitInfo& submit_info = submission.submit_infos.back();
          if (submit_info.command_buffers.empty()) {
            ++submit_info_reverse_it;
            continue;
          }
          // We found our last command buffer, so lets check if its result is there:
          const SubmittedCommandBuffer& last_command_buffer = submit_info.command_buffers.back();
          uint32_t check_slot_index_end = last_command_buffer.command_buffer_end_slot_index;

          static constexpr VkDeviceSize kResultStride = sizeof(uint64_t);
          uint64_t test_query_result = 0;
          VkResult query_worked = dispatch_table_->GetQueryPoolResults(device)(
              device, query_pool, check_slot_index_end, 1, sizeof(test_query_result),
              &test_query_result, kResultStride, VK_QUERY_RESULT_64_BIT);

          // Only erase the submission if we query its timers now.
          if (query_worked == VK_SUCCESS) {
            erase_submission = true;
            completed_submissions.push_back(submission);
          } else {
            erase_submission = false;
          }
          break;
        }

        if (erase_submission) {
          submission_it = queue_submissions.erase(submission_it);
        } else {
          ++submission_it;
        }
      }
    }

    // We sort the completed submissions by timestamp to avoid sending the capture events out of
    // order.
    std::sort(completed_submissions.begin(), completed_submissions.end(),
              [](const QueueSubmission& lhs, const QueueSubmission& rhs) {
                return lhs.meta_information.pre_submission_cpu_timestamp <
                       rhs.meta_information.pre_submission_cpu_timestamp;
              });

    return completed_submissions;
  }

  std::optional<uint64_t> QueryGpuTimestampNs(VkDevice device, VkQueryPool query_pool,
                                              uint32_t slot_index, float timestamp_period) {
    static constexpr VkDeviceSize kResultStride = sizeof(uint64_t);

    uint64_t timestamp = 0;
    VkResult result_status = dispatch_table_->GetQueryPoolResults(device)(
        device, query_pool, slot_index, 1, sizeof(timestamp), &timestamp, kResultStride,
        VK_QUERY_RESULT_64_BIT);

    if (result_status != VK_SUCCESS) {
      return std::nullopt;
    }

    return static_cast<uint64_t>(static_cast<double>(timestamp) * timestamp_period);
  }

  static void WriteMetaInfo(const SubmissionMetaInformation& meta_info,
                            orbit_grpc_protos::GpuQueueSubmissionMetaInfo* target_proto) {
    target_proto->set_tid(meta_info.thread_id);
    target_proto->set_pre_submission_cpu_timestamp(meta_info.pre_submission_cpu_timestamp);
    target_proto->set_post_submission_cpu_timestamp(meta_info.post_submission_cpu_timestamp);
  }

  [[nodiscard]] bool QuerySingleCommandBufferTimestamps(SubmittedCommandBuffer* command_buffer,
                                                        std::vector<uint32_t>* query_slots_to_reset,
                                                        VkDevice device, VkQueryPool query_pool,
                                                        float timestamp_period) {
    CHECK(command_buffer != nullptr);
    bool queries_succeeded = true;

    if (!command_buffer->end_timestamp.has_value()) {
      uint32_t slot_index = command_buffer->command_buffer_end_slot_index;
      std::optional<uint64_t> end_timestamp =
          QueryGpuTimestampNs(device, query_pool, slot_index, timestamp_period);
      if (end_timestamp.has_value()) {
        command_buffer->end_timestamp = end_timestamp;
        query_slots_to_reset->push_back(slot_index);
      } else {
        queries_succeeded = false;
      }
    }

    // It's possible that the command begin was not recorded, so we have to check if there is even
    // a query slot index for the begin timestamp before we try to query it.
    if (!command_buffer->command_buffer_begin_slot_index.has_value()) {
      return queries_succeeded;
    }

    if (!command_buffer->begin_timestamp.has_value()) {
      uint32_t slot_index = command_buffer->command_buffer_begin_slot_index.value();
      std::optional<uint64_t> begin_timestamp =
          QueryGpuTimestampNs(device, query_pool, slot_index, timestamp_period);
      if (begin_timestamp.has_value()) {
        command_buffer->begin_timestamp = begin_timestamp;
        query_slots_to_reset->push_back(slot_index);
      } else {
        queries_succeeded = false;
      }
    }

    return queries_succeeded;
  }

  [[nodiscard]] bool QueryCommandBufferTimestamps(QueueSubmission* completed_submission,
                                                  std::vector<uint32_t>* query_slots_to_reset,
                                                  VkDevice device, VkQueryPool query_pool,
                                                  float timestamp_period) {
    bool all_queries_succeeded = true;
    for (auto& completed_submit : completed_submission->submit_infos) {
      for (auto& completed_command_buffer : completed_submit.command_buffers) {
        all_queries_succeeded &= QuerySingleCommandBufferTimestamps(
            &completed_command_buffer, query_slots_to_reset, device, query_pool, timestamp_period);
      }
    }
    return all_queries_succeeded;
  }

  [[nodiscard]] bool QuerySingleDebugMarkerTimestamps(SubmittedMarkerSlice* marker_slice,
                                                      std::vector<uint32_t>* query_slots_to_reset,
                                                      VkDevice device, VkQueryPool query_pool,
                                                      float timestamp_period) {
    CHECK(marker_slice != nullptr);
    bool queries_succeeded = true;

    if (!marker_slice->end_info.timestamp.has_value()) {
      std::optional<uint64_t> end_timestamp = QueryGpuTimestampNs(
          device, query_pool, marker_slice->end_info.slot_index, timestamp_period);
      if (end_timestamp.has_value()) {
        marker_slice->end_info.timestamp = end_timestamp;
        query_slots_to_reset->push_back(marker_slice->end_info.slot_index);
      } else {
        queries_succeeded = false;
      }
    }

    if (!marker_slice->begin_info.has_value()) {
      return queries_succeeded;
    }

    if (!marker_slice->begin_info->timestamp.has_value()) {
      std::optional<uint64_t> begin_timestamp = QueryGpuTimestampNs(
          device, query_pool, marker_slice->begin_info->slot_index, timestamp_period);
      if (begin_timestamp.has_value()) {
        marker_slice->begin_info->timestamp = begin_timestamp;
        query_slots_to_reset->push_back(marker_slice->begin_info->slot_index);
      } else {
        queries_succeeded = false;
      }
    }
    return queries_succeeded;
  }

  [[nodiscard]] bool QueryDebugMarkerTimestamps(QueueSubmission* completed_submission,
                                                std::vector<uint32_t>* query_slots_to_reset,
                                                VkDevice device, VkQueryPool query_pool,
                                                float timestamp_period) {
    bool all_queries_succeeded = true;
    for (auto& marker_slice : completed_submission->completed_markers) {
      all_queries_succeeded &= QuerySingleDebugMarkerTimestamps(
          &marker_slice, query_slots_to_reset, device, query_pool, timestamp_period);
    }
    return all_queries_succeeded;
  }

  void WriteCommandBufferTimings(const QueueSubmission& completed_submission,
                                 orbit_grpc_protos::GpuQueueSubmission* submission_proto) {
    for (const auto& completed_submit : completed_submission.submit_infos) {
      orbit_grpc_protos::GpuSubmitInfo* submit_info_proto = submission_proto->add_submit_infos();
      for (const auto& completed_command_buffer : completed_submit.command_buffers) {
        orbit_grpc_protos::GpuCommandBuffer* command_buffer_proto =
            submit_info_proto->add_command_buffers();

        if (completed_command_buffer.command_buffer_begin_slot_index.has_value()) {
          // This function must only be called once queries have actually succeeded. If this command
          // buffer has a slot index for the begin timestamp, we must have a value here.
          CHECK(completed_command_buffer.begin_timestamp.has_value());
          command_buffer_proto->set_begin_gpu_timestamp_ns(
              completed_command_buffer.begin_timestamp.value());
        }

        // Similarly here, this function must only be called once timestamp queries have succeeded,
        // and therefore we must have an end timestamp here.
        CHECK(completed_command_buffer.end_timestamp.has_value());
        command_buffer_proto->set_end_gpu_timestamp_ns(
            completed_command_buffer.end_timestamp.value());
      }
    }
  }

  void WriteDebugMarkers(const QueueSubmission& completed_submission,
                         orbit_grpc_protos::GpuQueueSubmission* submission_proto) {
    submission_proto->set_num_begin_markers(completed_submission.num_begin_markers);
    for (const auto& marker_state : completed_submission.completed_markers) {
      CHECK(marker_state.end_info.timestamp.has_value());
      uint64_t end_timestamp = marker_state.end_info.timestamp.value();

      orbit_grpc_protos::GpuDebugMarker* marker_proto = submission_proto->add_completed_markers();
      if (vulkan_layer_producer_ != nullptr) {
        marker_proto->set_text_key(
            vulkan_layer_producer_->InternStringIfNecessaryAndGetKey(marker_state.label_name));
      }
      if (marker_state.color.red != 0.0f || marker_state.color.green != 0.0f ||
          marker_state.color.blue != 0.0f || marker_state.color.alpha != 0.0f) {
        auto color = marker_proto->mutable_color();
        color->set_red(marker_state.color.red);
        color->set_green(marker_state.color.green);
        color->set_blue(marker_state.color.blue);
        color->set_alpha(marker_state.color.alpha);
      }
      marker_proto->set_depth(marker_state.depth);
      marker_proto->set_end_gpu_timestamp_ns(end_timestamp);

      // If we haven't captured the begin marker, we'll leave the optional begin_marker empty.
      if (!marker_state.begin_info.has_value()) {
        continue;
      }
      orbit_grpc_protos::GpuDebugMarkerBeginInfo* begin_debug_marker_proto =
          marker_proto->mutable_begin_marker();
      WriteMetaInfo(marker_state.begin_info->meta_information,
                    begin_debug_marker_proto->mutable_meta_info());

      CHECK(marker_state.begin_info->timestamp.has_value());
      uint64_t begin_timestamp = marker_state.begin_info->timestamp.value();
      begin_debug_marker_proto->set_gpu_timestamp_ns(begin_timestamp);
    }
  }

  // This method does not acquire a lock and MUST NOT be called without holding the `mutex_`.
  void ResetCommandBufferUnsafe(VkCommandBuffer command_buffer) {
    if (!command_buffer_to_state_.contains(command_buffer)) {
      return;
    }
    CHECK(command_buffer_to_state_.contains(command_buffer));
    CommandBufferState& state = command_buffer_to_state_.at(command_buffer);
    CHECK(command_buffer_to_device_.contains(command_buffer));
    VkDevice device = command_buffer_to_device_.at(command_buffer);
    std::vector<uint32_t> marker_slots_to_rollback = {};
    if (state.command_buffer_begin_slot_index.has_value()) {
      marker_slots_to_rollback.push_back(state.command_buffer_begin_slot_index.value());
    }
    if (state.command_buffer_end_slot_index.has_value()) {
      marker_slots_to_rollback.push_back(state.command_buffer_end_slot_index.value());
    }
    for (const Marker& marker : state.markers) {
      if (marker.slot_index.has_value()) {
        marker_slots_to_rollback.push_back(marker.slot_index.value());
      }
    }
    timer_query_pool_->RollbackPendingQuerySlots(device, marker_slots_to_rollback);

    command_buffer_to_state_.erase(command_buffer);
  }

  absl::Mutex mutex_;
  absl::flat_hash_map<VkCommandPool, absl::flat_hash_set<VkCommandBuffer>> pool_to_command_buffers_;
  absl::flat_hash_map<VkCommandBuffer, VkDevice> command_buffer_to_device_;

  absl::flat_hash_map<VkCommandBuffer, CommandBufferState> command_buffer_to_state_;
  absl::flat_hash_map<VkQueue, std::vector<QueueSubmission>> queue_to_submissions_;

  absl::flat_hash_map<VkQueue, QueueMarkerState> queue_to_markers_;

  DispatchTable* dispatch_table_;
  TimerQueryPool* timer_query_pool_;
  DeviceManager* device_manager_;

  // We use std::numeric_limits<uint32_t>::max() to disable filtering of markers and 0 to discard
  // all debug markers.
  uint32_t max_local_marker_depth_per_command_buffer_ = std::numeric_limits<uint32_t>::max();

  [[nodiscard]] bool IsCapturing() {
    return vulkan_layer_producer_ != nullptr && vulkan_layer_producer_->IsCapturing();
  }
  VulkanLayerProducer* vulkan_layer_producer_;
};

}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_SUBMISSION_TRACKER_H_
