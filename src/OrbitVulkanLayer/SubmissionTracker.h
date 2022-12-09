// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_SUBMISSION_TRACKER_H_
#define ORBIT_VULKAN_LAYER_SUBMISSION_TRACKER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <vulkan/vulkan.h>

#include <functional>
#include <queue>
#include <stack>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"
#include "VulkanLayerProducer.h"

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
    uint32_t thread_id;
    uint32_t process_id;
  };

  // A persistent version of a command buffer that was submitted and its begin/end slot in the
  // `TimerQueryPool`. Note that the begin is optional, as it might not be part of the capture.
  // This struct is created if we capture the submission, so if we haven't captured the end,
  // we also haven't captured the begin, and don't need to store info about that command buffer at
  // all. The list of all `SubmittedCommandBuffer`s is stored in a `QueueSubmission`, and can so be
  // associated to the `SubmissionMetaInformation`.
  struct SubmittedCommandBuffer {
    std::optional<uint32_t> command_buffer_begin_slot_index;
    std::optional<uint32_t> command_buffer_end_slot_index;

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
    uint32_t slot_index = 0;

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
    size_t depth = 0;
  };

  // A single submission (VkQueueSubmit) can contain multiple `SubmitInfo`s. We keep this structure.
  struct SubmitInfo {
    std::vector<SubmittedCommandBuffer> command_buffers;
  };

  // Wraps up all the data that needs to be persistent upon a submission (VkQueueSubmit).
  // `completed_markers` are all the debug markers that got completed (via "End") within this
  // submission. Their "Begin" might still be in a different submission.
  struct QueueSubmission {
    VkQueue queue{};
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
    ORBIT_CHECK(dispatch_table_ != nullptr);
    ORBIT_CHECK(timer_query_pool_ != nullptr);
    ORBIT_CHECK(device_manager_ != nullptr);
  }

  // Sets a producer to be used to enqueue capture events and asks if we are capturing.
  // We will also register ourselves as CaptureStatusListener, in order to get notified on
  // capture finish (OnCaptureFinished). That's when we will reset the open query slots.
  void SetVulkanLayerProducer(VulkanLayerProducer* vulkan_layer_producer) {
    // De-register from the previous VulkanLayerProducer.
    if (vulkan_layer_producer_ != nullptr) {
      vulkan_layer_producer_->SetCaptureStatusListener(nullptr);
    }
    vulkan_layer_producer_ = vulkan_layer_producer;
    // Register to the new VulkanLayerProducer.
    if (vulkan_layer_producer_ != nullptr) {
      vulkan_layer_producer_->SetCaptureStatusListener(this);
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
    ORBIT_CHECK(pool_to_command_buffers_.contains(pool));
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
        // Note: This will "rollback" the slot indices (rather then actually resetting them on the
        // Gpu). This is fine, as we remove the command buffer state right after submission. Thus,
        // There can not be a value in the respective slot.
        ResetCommandBufferUnsafe(command_buffer);

        command_buffer_to_state_.erase(command_buffer);
      }

      ORBIT_CHECK(command_buffer_to_device_.contains(command_buffer));
      ORBIT_CHECK(command_buffer_to_device_.at(command_buffer) == device);
      command_buffer_to_device_.erase(command_buffer);
    }
    if (associated_command_buffers.empty()) {
      pool_to_command_buffers_.erase(pool);
    }
  }

  void MarkCommandBufferBegin(VkCommandBuffer command_buffer) {
    absl::WriterMutexLock lock(&mutex_);
    // Even when we are not capturing we create state for this command buffer to allow the
    // debug marker tracking. In order to compute the correct depth of a debug marker and being able
    // to match an "end" marker with the corresponding "begin" marker, we maintain a stack of all
    // debug markers of a queue. The order of the markers is determined upon submission based on the
    // order within the submitted command buffers. Thus, even when not capturing, we create an empty
    // state here that allows us to store the debug markers into it and maintain that stack on
    // submission. We will not write timestamps in this case and thus don't store any information
    // other than the debug markers then.
    {
      if (command_buffer_to_state_.contains(command_buffer)) {
        // We end up in this case, if we have used the command buffer before and want to write new
        // commands to it without resetting the command buffer. Per specification,
        // "vkBeginCommandBuffer" does also reset the command buffer, in addition to putting it
        // into the executable state.
        ResetCommandBufferUnsafe(command_buffer);
      }
      command_buffer_to_state_[command_buffer] = {};
    }
    if (!is_capturing_) {
      return;
    }

    uint32_t slot_index;
    if (RecordTimestamp(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, &slot_index)) {
      ORBIT_CHECK(command_buffer_to_state_.contains(command_buffer));
      command_buffer_to_state_.at(command_buffer).command_buffer_begin_slot_index =
          std::make_optional(slot_index);
    }
  }

  void MarkCommandBufferEnd(VkCommandBuffer command_buffer) {
    absl::WriterMutexLock lock(&mutex_);
    if (!is_capturing_) {
      return;
    }
    if (!command_buffer_to_state_.contains(command_buffer)) {
      ORBIT_ERROR_ONCE(
          "Calling vkEndCommandBuffer on a command buffer that is in the initial state "
          "(i.e. either freshly allocated or reset with vkResetCommandBuffer).");
      return;
    }

    uint32_t slot_index;
    if (RecordTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, &slot_index)) {
      // MarkCommandBufferBegin/End are called from within the same submit, and as the
      // `MarkCommandBufferBegin` will always insert the state, we can assume that it is there.
      ORBIT_CHECK(command_buffer_to_state_.contains(command_buffer));
      CommandBufferState& command_buffer_state = command_buffer_to_state_.at(command_buffer);
      command_buffer_state.command_buffer_end_slot_index = std::make_optional(slot_index);
    }
  }

  void MarkDebugMarkerBegin(VkCommandBuffer command_buffer, const char* text, Color color) {
    absl::WriterMutexLock lock(&mutex_);
    // It is ensured by the Vulkan spec. that `text` must not be nullptr.
    ORBIT_CHECK(text != nullptr);
    bool marker_depth_exceeds_maximum;
    {
      if (!command_buffer_to_state_.contains(command_buffer)) {
        ORBIT_ERROR_ONCE(
            "Calling vkCmdDebugMarkerBeginEXT/vkCmdBeginDebugUtilsLabelEXT on a command buffer "
            "that is in the initial state (i.e. either freshly allocated or reset with "
            "vkResetCommandBuffer).");
        return;
      }
      ORBIT_CHECK(command_buffer_to_state_.contains(command_buffer));
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

    if (!is_capturing_ || marker_depth_exceeds_maximum) {
      return;
    }

    uint32_t slot_index;
    if (RecordTimestamp(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, &slot_index)) {
      ORBIT_CHECK(command_buffer_to_state_.contains(command_buffer));
      CommandBufferState& state = command_buffer_to_state_.at(command_buffer);
      state.markers.back().slot_index = std::make_optional(slot_index);
    }
  }

  void MarkDebugMarkerEnd(VkCommandBuffer command_buffer) {
    absl::WriterMutexLock lock(&mutex_);
    bool marker_depth_exceeds_maximum;

    if (!command_buffer_to_state_.contains(command_buffer)) {
      ORBIT_ERROR_ONCE(
          "Calling vkCmdDebugMarkerEndEXT/vkCmdEndDebugUtilsLabelEXT on a command buffer "
          "that is in the initial state (i.e. either freshly allocated or reset with "
          "vkResetCommandBuffer).");
      return;
    }
    ORBIT_CHECK(command_buffer_to_state_.contains(command_buffer));
    CommandBufferState& state = command_buffer_to_state_.at(command_buffer);
    marker_depth_exceeds_maximum =
        state.local_marker_stack_size > max_local_marker_depth_per_command_buffer_;
    Marker marker{.type = MarkerType::kDebugMarkerEnd, .cut_off = marker_depth_exceeds_maximum};
    state.markers.emplace_back(std::move(marker));
    // We might see more "ends" than "begins", as the "begins" can be on a different command
    // buffer.
    if (state.local_marker_stack_size > 0) {
      --state.local_marker_stack_size;
    }

    if (!is_capturing_ || marker_depth_exceeds_maximum) {
      return;
    }

    uint32_t slot_index;
    if (RecordTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, &slot_index)) {
      ORBIT_CHECK(command_buffer_to_state_.contains(command_buffer));
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
    absl::WriterMutexLock lock(&mutex_);
    if (!is_capturing_) {
      // `OnCaptureFinished` has already been called and has taken care of resetting slots.
      return std::nullopt;
    }

    VkDevice device = VK_NULL_HANDLE;
    // Collect slots of command buffer "begins" that are baked into the command buffer but for which
    // the "end" is missing. We will never read those, but need to wait until the command buffer
    // gets reset.
    std::vector<uint32_t> query_slots_not_needed_to_read;

    QueueSubmission queue_submission;
    queue_submission.queue = queue;
    queue_submission.meta_information.pre_submission_cpu_timestamp =
        orbit_base::CaptureTimestampNs();
    queue_submission.meta_information.thread_id = orbit_base::GetCurrentThreadId();
    queue_submission.meta_information.process_id = orbit_base::GetCurrentProcessId();

    for (uint32_t submit_index = 0; submit_index < submit_count; ++submit_index) {
      VkSubmitInfo submit_info = submits[submit_index];
      queue_submission.submit_infos.emplace_back();
      SubmitInfo& submitted_submit_info = queue_submission.submit_infos.back();
      for (uint32_t command_buffer_index = 0; command_buffer_index < submit_info.commandBufferCount;
           ++command_buffer_index) {
        VkCommandBuffer command_buffer = submit_info.pCommandBuffers[command_buffer_index];
        PersistSingleCommandBufferOnSubmit(device, command_buffer, &queue_submission,
                                           &submitted_submit_info, &query_slots_not_needed_to_read);
      }
    }

    // Slots here will not be reset twice, but remain marked as unavailable until the command
    // buffers are done executing and "vkResetCommandBuffer" was called. The slots are still baked
    // into the command buffers that were sent to the GPU. Using the same slots again would likely
    // lead to wrong results.
    if (!query_slots_not_needed_to_read.empty()) {
      timer_query_pool_->MarkQuerySlotsDoneReading(device, query_slots_not_needed_to_read);
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
    ORBIT_CHECK(queue_to_markers_.contains(queue));
    QueueMarkerState& markers = queue_to_markers_.at(queue);

    // If we consider that we are still capturing, take a cpu timestamp as "post submission" such
    // that the submission "meta information" is complete. We can then attach that also to each
    // debug marker, as they may span across different submissions.
    if (queue_submission_optional.has_value()) {
      queue_submission_optional->meta_information.post_submission_cpu_timestamp =
          orbit_base::CaptureTimestampNs();
    }

    std::vector<uint32_t> marker_slots_not_needed_to_read;
    VkDevice device = VK_NULL_HANDLE;
    for (uint32_t submit_index = 0; submit_index < submit_count; ++submit_index) {
      VkSubmitInfo submit_info = submits[submit_index];
      for (uint32_t command_buffer_index = 0; command_buffer_index < submit_info.commandBufferCount;
           ++command_buffer_index) {
        VkCommandBuffer command_buffer = submit_info.pCommandBuffers[command_buffer_index];
        if (device == VK_NULL_HANDLE) {
          ORBIT_CHECK(command_buffer_to_device_.contains(command_buffer));
          device = command_buffer_to_device_.at(command_buffer);
        }
        PersistDebugMarkersOfASingleCommandBufferOnSubmit(
            command_buffer, &queue_submission_optional, &markers, &marker_slots_not_needed_to_read);
      }
    }

    if (!marker_slots_not_needed_to_read.empty()) {
      timer_query_pool_->MarkQuerySlotsDoneReading(device, marker_slots_not_needed_to_read);
    }

    if (!queue_submission_optional.has_value()) {
      return;
    }

    if (!queue_to_submission_priority_queue_.contains(queue)) {
      queue_to_submission_priority_queue_.emplace(queue, kPreSubmissionCpuTimestampComparator);
    }

    queue_to_submission_priority_queue_.at(queue).emplace(
        std::move(queue_submission_optional.value()));
  }

  // This method is responsible for retrieving all the timestamps for the "completed" submissions,
  // for transforming the information of those submissions (in particular about the command buffers
  // and debug markers) into the `GpuQueueSubmission` proto, and for sending it to the
  // `VulkanLayerProducer`. We consider a submission to be "completed" when all timestamps that are
  // associated with this submission are ready.
  // We maintain a priority queue (for every `VkQueue`) `submissions_queue_` and process submissions
  // with the oldest CPU timestamp, until we encounter the first "incomplete" submission.
  // This way, we ensure that we will send the submission information per queue ordered by the
  // CPU timestamp.
  // Beside the timestamps of command buffers and the meta information of the submission, the proto
  // also contains the debug markers, "begin" (even if submitted in a different submission) and
  // "end", that got completed in this submission.
  // See also `WriteMetaInfo`, `WriteCommandBufferTimings` and `WriteDebugMarkers`.
  // This method also resets all the timer slots that have been read.
  // It is assumed to be called periodically, e.g. on `vkQueuePresentKHR`.
  void CompleteSubmits(VkDevice device) {
    absl::WriterMutexLock lock(&mutex_);
    VkQueryPool query_pool = timer_query_pool_->GetQueryPool(device);

    if (queue_to_submission_priority_queue_.empty()) {
      return;
    }

    VkPhysicalDevice physical_device = device_manager_->GetPhysicalDeviceOfLogicalDevice(device);
    const float timestamp_period =
        device_manager_->GetPhysicalDeviceProperties(physical_device).limits.timestampPeriod;

    std::vector<uint32_t> query_slots_done_reading = {};
    std::vector<QueueSubmission> submissions_to_send = {};

    // The submits of a specific queue in `submissions_queue_` are sorted by "pre submission CPU"
    // timestamp and we want to make sure we send events to the client in that order. Therefore, we
    // stop as soon as a query failed.
    for (auto& [unused_queue, submissions] : queue_to_submission_priority_queue_) {
      while (!submissions.empty()) {
        QueueSubmission completed_submission = submissions.top();
        submissions.pop();
        bool command_buffer_queries_succeeded = QueryCommandBufferTimestamps(
            &completed_submission, &query_slots_done_reading, device, query_pool, timestamp_period);

        // We only need to read the debug marker timestamps, if querying the command buffers
        // succeeded.
        bool marker_queries_succeeded = false;
        if (command_buffer_queries_succeeded) {
          marker_queries_succeeded =
              QueryDebugMarkerTimestamps(&completed_submission, &query_slots_done_reading, device,
                                         query_pool, timestamp_period);
        }

        if (command_buffer_queries_succeeded && marker_queries_succeeded) {
          submissions_to_send.emplace_back(std::move(completed_submission));
        } else {
          submissions.emplace(std::move(completed_submission));
          break;
        }
      }
    }

    for (const auto& completed_submission : submissions_to_send) {
      orbit_grpc_protos::ProducerCaptureEvent capture_event;
      orbit_grpc_protos::GpuQueueSubmission* submission_proto =
          capture_event.mutable_gpu_queue_submission();

      WriteMetaInfo(completed_submission.meta_information, submission_proto->mutable_meta_info());
      bool has_command_buffer_timestamps =
          WriteCommandBufferTimings(completed_submission, submission_proto);
      bool has_debug_marker_timestamps = WriteDebugMarkers(completed_submission, submission_proto);

      if (vulkan_layer_producer_ != nullptr &&
          (has_command_buffer_timestamps || has_debug_marker_timestamps)) {
        vulkan_layer_producer_->EnqueueCaptureEvent(std::move(capture_event));
      }
    }

    if (!query_slots_done_reading.empty()) {
      timer_query_pool_->MarkQuerySlotsDoneReading(device, query_slots_done_reading);
    }
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
      ORBIT_CHECK(pool_to_command_buffers_.contains(command_pool));
      command_buffers = pool_to_command_buffers_.at(command_pool);
    }
    for (const auto& command_buffer : command_buffers) {
      ResetCommandBuffer(command_buffer);
    }
  }

  void OnCaptureStart(orbit_grpc_protos::CaptureOptions capture_options) override {
    absl::WriterMutexLock lock(&mutex_);
    SetMaxLocalMarkerDepthPerCommandBuffer(
        capture_options.max_local_marker_depth_per_command_buffer());
    is_capturing_ = true;
  }

  void OnCaptureStop() override {}

  void OnCaptureFinished() override {
    absl::WriterMutexLock lock(&mutex_);
    std::vector<uint32_t> slots_not_needed_to_read_anymore;

    VkDevice device = VK_NULL_HANDLE;

    for (auto& [command_buffer, command_buffer_state] : command_buffer_to_state_) {
      if (command_buffer_state.pre_submission_cpu_timestamp.has_value()) continue;
      if (device == VK_NULL_HANDLE) {
        ORBIT_CHECK(command_buffer_to_device_.contains(command_buffer));
        device = command_buffer_to_device_.at(command_buffer);
      }
      if (command_buffer_state.command_buffer_begin_slot_index.has_value()) {
        slots_not_needed_to_read_anymore.push_back(
            command_buffer_state.command_buffer_begin_slot_index.value());
        command_buffer_state.command_buffer_begin_slot_index.reset();
      }

      if (command_buffer_state.command_buffer_end_slot_index.has_value()) {
        slots_not_needed_to_read_anymore.push_back(
            command_buffer_state.command_buffer_end_slot_index.value());
        command_buffer_state.command_buffer_end_slot_index.reset();
      }

      for (Marker& marker : command_buffer_state.markers) {
        if (marker.slot_index.has_value()) {
          slots_not_needed_to_read_anymore.push_back(marker.slot_index.value());
          marker.slot_index.reset();
        }
      }
    }
    if (!slots_not_needed_to_read_anymore.empty()) {
      timer_query_pool_->MarkQuerySlotsDoneReading(device, slots_not_needed_to_read_anymore);
    }

    is_capturing_ = false;
  }

 private:
  enum class MarkerType { kDebugMarkerBegin = 0, kDebugMarkerEnd };

  struct Marker {
    MarkerType type;
    std::optional<uint32_t> slot_index;
    std::optional<std::string> label_name;
    std::optional<Color> color;
    bool cut_off = false;
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
    size_t depth = 0;
    bool depth_exceeds_maximum = false;
  };

  struct QueueMarkerState {
    std::stack<MarkerState> marker_stack;
  };

  struct CommandBufferState {
    std::optional<uint32_t> command_buffer_begin_slot_index;
    std::optional<uint32_t> command_buffer_end_slot_index;
    std::optional<uint64_t> pre_submission_cpu_timestamp;
    std::vector<Marker> markers;
    uint32_t local_marker_stack_size = 0;
  };

  bool RecordTimestamp(VkCommandBuffer command_buffer, VkPipelineStageFlagBits pipeline_stage_flags,
                       uint32_t* slot_index) {
    mutex_.AssertReaderHeld();
    VkDevice device;
    {
      ORBIT_CHECK(command_buffer_to_device_.contains(command_buffer));
      device = command_buffer_to_device_.at(command_buffer);
    }

    VkQueryPool query_pool = timer_query_pool_->GetQueryPool(device);

    if (!timer_query_pool_->NextReadyQuerySlot(device, slot_index)) {
      // TODO(b/192998580): Send an appropriate CaptureEvent to notify the client that the Vulkan
      //  layer was out of query slots in a certain time range.
      return false;
    }
    dispatch_table_->CmdWriteTimestamp(command_buffer)(command_buffer, pipeline_stage_flags,
                                                       query_pool, *slot_index);

    return true;
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
    target_proto->set_pid(meta_info.process_id);
    target_proto->set_pre_submission_cpu_timestamp(meta_info.pre_submission_cpu_timestamp);
    target_proto->set_post_submission_cpu_timestamp(meta_info.post_submission_cpu_timestamp);
  }

  [[nodiscard]] bool QuerySingleCommandBufferTimestamps(SubmittedCommandBuffer* command_buffer,
                                                        std::vector<uint32_t>* query_slots_to_reset,
                                                        VkDevice device, VkQueryPool query_pool,
                                                        float timestamp_period) {
    ORBIT_CHECK(command_buffer != nullptr);

    if (!command_buffer->end_timestamp.has_value()) {
      ORBIT_CHECK(command_buffer->command_buffer_end_slot_index.has_value());
      uint32_t slot_index = command_buffer->command_buffer_end_slot_index.value();
      std::optional<uint64_t> end_timestamp =
          QueryGpuTimestampNs(device, query_pool, slot_index, timestamp_period);
      if (end_timestamp.has_value()) {
        command_buffer->end_timestamp = end_timestamp;
        query_slots_to_reset->push_back(slot_index);
      } else {
        return false;
      }
    }

    // It's possible that the command begin was not recorded, so we have to check if there is even
    // a query slot index for the begin timestamp before we try to query it.
    if (!command_buffer->command_buffer_begin_slot_index.has_value()) {
      return true;
    }

    if (!command_buffer->begin_timestamp.has_value()) {
      uint32_t slot_index = command_buffer->command_buffer_begin_slot_index.value();
      std::optional<uint64_t> begin_timestamp =
          QueryGpuTimestampNs(device, query_pool, slot_index, timestamp_period);
      if (begin_timestamp.has_value()) {
        command_buffer->begin_timestamp = begin_timestamp;
        query_slots_to_reset->push_back(slot_index);
      } else {
        return false;
      }
    }

    return true;
  }

  [[nodiscard]] bool QueryCommandBufferTimestamps(QueueSubmission* completed_submission,
                                                  std::vector<uint32_t>* query_slots_to_reset,
                                                  VkDevice device, VkQueryPool query_pool,
                                                  float timestamp_period) {
    for (auto& completed_submit : completed_submission->submit_infos) {
      for (auto& completed_command_buffer : completed_submit.command_buffers) {
        bool queries_succeeded = QuerySingleCommandBufferTimestamps(
            &completed_command_buffer, query_slots_to_reset, device, query_pool, timestamp_period);
        if (!queries_succeeded) return false;
      }
    }
    return true;
  }

  [[nodiscard]] bool QuerySingleDebugMarkerTimestamps(SubmittedMarkerSlice* marker_slice,
                                                      std::vector<uint32_t>* query_slots_to_reset,
                                                      VkDevice device, VkQueryPool query_pool,
                                                      float timestamp_period) {
    ORBIT_CHECK(marker_slice != nullptr);

    if (!marker_slice->end_info.timestamp.has_value()) {
      std::optional<uint64_t> end_timestamp = QueryGpuTimestampNs(
          device, query_pool, marker_slice->end_info.slot_index, timestamp_period);
      if (end_timestamp.has_value()) {
        marker_slice->end_info.timestamp = end_timestamp;
        query_slots_to_reset->push_back(marker_slice->end_info.slot_index);
      } else {
        return false;
      }
    }

    if (!marker_slice->begin_info.has_value()) {
      return true;
    }

    if (!marker_slice->begin_info->timestamp.has_value()) {
      std::optional<uint64_t> begin_timestamp = QueryGpuTimestampNs(
          device, query_pool, marker_slice->begin_info->slot_index, timestamp_period);
      if (begin_timestamp.has_value()) {
        marker_slice->begin_info->timestamp = begin_timestamp;
        query_slots_to_reset->push_back(marker_slice->begin_info->slot_index);
      } else {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] bool QueryDebugMarkerTimestamps(QueueSubmission* completed_submission,
                                                std::vector<uint32_t>* query_slots_to_reset,
                                                VkDevice device, VkQueryPool query_pool,
                                                float timestamp_period) {
    for (auto& marker_slice : completed_submission->completed_markers) {
      bool queries_succeeded = QuerySingleDebugMarkerTimestamps(
          &marker_slice, query_slots_to_reset, device, query_pool, timestamp_period);
      if (!queries_succeeded) return false;
    }
    return true;
  }

  [[nodiscard]] bool WriteCommandBufferTimings(
      const QueueSubmission& completed_submission,
      orbit_grpc_protos::GpuQueueSubmission* submission_proto) {
    bool has_at_least_one_timestamp = false;
    for (const auto& completed_submit : completed_submission.submit_infos) {
      orbit_grpc_protos::GpuSubmitInfo* submit_info_proto = submission_proto->add_submit_infos();
      for (const auto& completed_command_buffer : completed_submit.command_buffers) {
        orbit_grpc_protos::GpuCommandBuffer* command_buffer_proto =
            submit_info_proto->add_command_buffers();

        if (completed_command_buffer.command_buffer_begin_slot_index.has_value()) {
          // This function must only be called once queries have actually succeeded. If this command
          // buffer has a slot index for the begin timestamp, we must have a value here.
          ORBIT_CHECK(completed_command_buffer.begin_timestamp.has_value());
          command_buffer_proto->set_begin_gpu_timestamp_ns(
              completed_command_buffer.begin_timestamp.value());
        }

        // Similarly here, this function must only be called once timestamp queries have succeeded,
        // and therefore we must have an end timestamp here.
        ORBIT_CHECK(completed_command_buffer.end_timestamp.has_value());
        command_buffer_proto->set_end_gpu_timestamp_ns(
            completed_command_buffer.end_timestamp.value());

        has_at_least_one_timestamp = true;
      }
    }
    return has_at_least_one_timestamp;
  }

  [[nodiscard]] bool WriteDebugMarkers(const QueueSubmission& completed_submission,
                                       orbit_grpc_protos::GpuQueueSubmission* submission_proto) {
    submission_proto->set_num_begin_markers(completed_submission.num_begin_markers);
    bool has_at_least_one_timestamp = false;
    for (const auto& marker_state : completed_submission.completed_markers) {
      ORBIT_CHECK(marker_state.end_info.timestamp.has_value());
      uint64_t end_timestamp = marker_state.end_info.timestamp.value();
      has_at_least_one_timestamp = true;

      orbit_grpc_protos::GpuDebugMarker* marker_proto = submission_proto->add_completed_markers();
      if (vulkan_layer_producer_ != nullptr) {
        marker_proto->set_text_key(
            vulkan_layer_producer_->InternStringIfNecessaryAndGetKey(marker_state.label_name));
      }

      auto quantize = [](float value) { return static_cast<uint8_t>(value * 255.f); };

      // We have seen near 0.f color values in all four components "in the wild", so we check if the
      // quantized values are not zero here to make sure these markers are rendered with an actual
      // color.
      if (quantize(marker_state.color.red) != 0 || quantize(marker_state.color.green) != 0 ||
          quantize(marker_state.color.blue) != 0 || quantize(marker_state.color.alpha) != 0) {
        auto* color = marker_proto->mutable_color();
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

      ORBIT_CHECK(marker_state.begin_info->timestamp.has_value());
      uint64_t begin_timestamp = marker_state.begin_info->timestamp.value();
      begin_debug_marker_proto->set_gpu_timestamp_ns(begin_timestamp);
    }

    return has_at_least_one_timestamp;
  }

  // This method does not acquire a lock and MUST NOT be called without holding the `mutex_`.
  void ResetCommandBufferUnsafe(VkCommandBuffer command_buffer) {
    mutex_.AssertHeld();
    if (!command_buffer_to_state_.contains(command_buffer)) {
      return;
    }
    ORBIT_CHECK(command_buffer_to_state_.contains(command_buffer));
    CommandBufferState& state = command_buffer_to_state_.at(command_buffer);
    ORBIT_CHECK(command_buffer_to_device_.contains(command_buffer));
    VkDevice device = command_buffer_to_device_.at(command_buffer);
    std::vector<uint32_t> query_slots_to_reset{};
    if (state.command_buffer_begin_slot_index.has_value()) {
      query_slots_to_reset.push_back(state.command_buffer_begin_slot_index.value());
    }
    if (state.command_buffer_end_slot_index.has_value()) {
      query_slots_to_reset.push_back(state.command_buffer_end_slot_index.value());
    }
    for (const Marker& marker : state.markers) {
      if (marker.slot_index.has_value()) {
        query_slots_to_reset.push_back(marker.slot_index.value());
      }
    }
    if (state.pre_submission_cpu_timestamp.has_value()) {
      timer_query_pool_->MarkQuerySlotsForReset(device, query_slots_to_reset);
    } else {
      timer_query_pool_->RollbackPendingQuerySlots(device, query_slots_to_reset);
    }

    command_buffer_to_state_.erase(command_buffer);
  }

  void PersistSingleCommandBufferOnSubmit(VkDevice device, VkCommandBuffer command_buffer,
                                          QueueSubmission* queue_submission,
                                          SubmitInfo* submitted_submit_info,
                                          std::vector<uint32_t>* query_slots_not_needed_to_read) {
    mutex_.AssertHeld();
    ORBIT_CHECK(queue_submission != nullptr);
    ORBIT_CHECK(submitted_submit_info != nullptr);
    ORBIT_CHECK(query_slots_not_needed_to_read != nullptr);

    if (!command_buffer_to_state_.contains(command_buffer)) {
      ORBIT_ERROR_ONCE(
          "Calling vkQueueSubmit on a command buffer that is in the initial state (i.e. "
          "either freshly allocated or reset with vkResetCommandBuffer).");
      return;
    }
    ORBIT_CHECK(command_buffer_to_state_.contains(command_buffer));
    CommandBufferState& state = command_buffer_to_state_.at(command_buffer);
    bool has_been_submitted_before = state.pre_submission_cpu_timestamp.has_value();

    // Mark that this command buffer in the current state was already submitted. If the command
    // buffer is being submitted again, without a "vkResetCommandBuffer" call, the same slot
    // indices will be used again, so that we can not distinguish the results for the different
    // submissions anymore. Thus, this field will be used to ensure to not try to read such a
    // slot. Calling "vkResetCommandBuffer" (or "vkBeginCommandBuffer"), will reset this field.
    // Note that we are using an std::optional with the submission time to protect us against an
    // "OnCaptureFinished" call, right between pre and post submission, which would try to reset
    // the slots.
    state.pre_submission_cpu_timestamp =
        queue_submission->meta_information.pre_submission_cpu_timestamp;

    if (device == VK_NULL_HANDLE) {
      device = command_buffer_to_device_.at(command_buffer);
    }

    // If we haven't recorded neither the end nor the begin of a command buffer, we have no
    // information to send.
    if (!state.command_buffer_end_slot_index.has_value()) {
      if (state.command_buffer_begin_slot_index.has_value()) {
        // We need to discard the begin slot when we only have a begin slot. This can happen
        // if we run out of query slots.
        query_slots_not_needed_to_read->push_back(state.command_buffer_begin_slot_index.value());
        state.command_buffer_begin_slot_index.reset();
      }
      return;
    }

    // If this command buffer was already submitted before (without reset afterwards), its slot
    // indices are not unique anymore and can't be used by us. Note, that the slots will be
    // marked as "done with reading" when the first submission of this command buffer is done
    // and will eventually be reset on a "vkResetCommandBuffer" call.
    if (!has_been_submitted_before) {
      SubmittedCommandBuffer submitted_command_buffer{
          .command_buffer_begin_slot_index = state.command_buffer_begin_slot_index,
          .command_buffer_end_slot_index = state.command_buffer_end_slot_index.value()};
      submitted_submit_info->command_buffers.emplace_back(submitted_command_buffer);
    }
  }

  void PersistDebugMarkersOfASingleCommandBufferOnSubmit(
      VkCommandBuffer command_buffer, std::optional<QueueSubmission>* queue_submission_optional,
      QueueMarkerState* markers, std::vector<uint32_t>* marker_slots_not_needed_to_read) {
    mutex_.AssertHeld();
    ORBIT_CHECK(queue_submission_optional != nullptr);
    ORBIT_CHECK(markers != nullptr);
    ORBIT_CHECK(marker_slots_not_needed_to_read != nullptr);

    if (!command_buffer_to_state_.contains(command_buffer)) {
      ORBIT_ERROR_ONCE(
          "Calling vkQueueSubmit on a command buffer that is in the initial state (i.e. "
          "either freshly allocated or reset with vkResetCommandBuffer).");
      return;
    }
    ORBIT_CHECK(command_buffer_to_state_.contains(command_buffer));
    const CommandBufferState& state = command_buffer_to_state_.at(command_buffer);

    for (const Marker& marker : state.markers) {
      std::optional<SubmittedMarker> submitted_marker = std::nullopt;
      ORBIT_CHECK(!queue_submission_optional->has_value() ||
                  state.pre_submission_cpu_timestamp.has_value());
      if (marker.slot_index.has_value() && queue_submission_optional->has_value() &&
          (state.pre_submission_cpu_timestamp.value() ==
           queue_submission_optional->value().meta_information.pre_submission_cpu_timestamp)) {
        submitted_marker = {.meta_information = queue_submission_optional->value().meta_information,
                            .slot_index = marker.slot_index.value()};
      }

      switch (marker.type) {
        case MarkerType::kDebugMarkerBegin: {
          if (queue_submission_optional->has_value() && marker.slot_index.has_value()) {
            ++queue_submission_optional->value().num_begin_markers;
          }
          ORBIT_CHECK(marker.label_name.has_value());
          ORBIT_CHECK(marker.color.has_value());
          MarkerState marker_state{.begin_info = submitted_marker,
                                   .label_name = marker.label_name.value(),
                                   .color = marker.color.value(),
                                   .depth = markers->marker_stack.size(),
                                   .depth_exceeds_maximum = marker.cut_off};
          markers->marker_stack.push(std::move(marker_state));
          break;
        }

        case MarkerType::kDebugMarkerEnd: {
          MarkerState marker_state = markers->marker_stack.top();
          markers->marker_stack.pop();

          // If there is a begin marker slot from a previous submission, this is our chance to
          // state that we will not read the slot anymore. We can reset is as soon as the
          // command buffer itself gets reset.
          if (marker_state.begin_info.has_value() && !queue_submission_optional->has_value()) {
            marker_slots_not_needed_to_read->push_back(marker_state.begin_info->slot_index);
          }

          // If the begin marker was discarded for exceeding the maximum depth, but the end
          // marker was not (as it is in a different submission), we will not read the slot in
          // "CompleteSubmits". We can reset is as soon as the command buffer itself gets reset.
          if (marker_state.depth_exceeds_maximum && marker.slot_index.has_value()) {
            marker_slots_not_needed_to_read->push_back(marker.slot_index.value());
          }

          if (queue_submission_optional->has_value() && marker.slot_index.has_value() &&
              !marker_state.depth_exceeds_maximum) {
            ORBIT_CHECK(submitted_marker.has_value());
            queue_submission_optional->value().completed_markers.emplace_back(
                SubmittedMarkerSlice{.begin_info = marker_state.begin_info,
                                     .end_info = submitted_marker.value(),
                                     .label_name = std::move(marker_state.label_name),
                                     .color = marker_state.color,
                                     .depth = marker_state.depth});
          }

          break;
        }
      }
    }
  }

  absl::Mutex mutex_;
  absl::flat_hash_map<VkCommandPool, absl::flat_hash_set<VkCommandBuffer>> pool_to_command_buffers_;
  absl::flat_hash_map<VkCommandBuffer, VkDevice> command_buffer_to_device_;

  absl::flat_hash_map<VkCommandBuffer, CommandBufferState> command_buffer_to_state_;

  static constexpr auto kPreSubmissionCpuTimestampComparator =
      [](const QueueSubmission& lhs, const QueueSubmission& rhs) -> bool {
    return lhs.meta_information.pre_submission_cpu_timestamp >
           rhs.meta_information.pre_submission_cpu_timestamp;
  };
  absl::flat_hash_map<VkQueue,
                      std::priority_queue<QueueSubmission, std::vector<QueueSubmission>,
                                          std::function<bool(QueueSubmission, QueueSubmission)>>>
      queue_to_submission_priority_queue_;

  absl::flat_hash_map<VkQueue, QueueMarkerState> queue_to_markers_;

  DispatchTable* dispatch_table_;
  TimerQueryPool* timer_query_pool_;
  DeviceManager* device_manager_;

  // We use std::numeric_limits<uint32_t>::max() to disable filtering of markers and 0 to discard
  // all debug markers.
  uint32_t max_local_marker_depth_per_command_buffer_ = std::numeric_limits<uint32_t>::max();
  VulkanLayerProducer* vulkan_layer_producer_ = nullptr;

  // This boolean is precisely true between a call to OnCaptureStart and OnCaptureFinished. In
  // particular, OnCaptureFinished changes command buffer state and resets query slots, and we
  // must ensure state remains consistent with respect to calls to marking begins and ends of
  // command buffers and debug markers. A consistent state allows proper cleanup of query slots
  // either in OnCaptureFinished or when completing submits. Note that calling
  // vulkan_layer_producer_->IsCapturing() is not a correct replacement for checking this boolean.
  bool is_capturing_ = false;
};

}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_SUBMISSION_TRACKER_H_
