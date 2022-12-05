// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "CaptureClient/GpuQueueSubmissionProcessor.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"

using google::protobuf::util::MessageDifferencer;
using orbit_client_protos::TimerInfo;
using orbit_client_protos::TimerInfo_Type_kGpuCommandBuffer;
using orbit_grpc_protos::Color;
using orbit_grpc_protos::GpuCommandBuffer;
using orbit_grpc_protos::GpuDebugMarker;
using orbit_grpc_protos::GpuDebugMarkerBeginInfo;
using orbit_grpc_protos::GpuJob;
using orbit_grpc_protos::GpuQueueSubmission;
using orbit_grpc_protos::GpuQueueSubmissionMetaInfo;
using orbit_grpc_protos::GpuSubmitInfo;

namespace orbit_capture_client {

class GpuQueueSubmissionProcessorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    string_intern_pool_[kTimelineKey] = kTimeline;
    string_intern_pool_[kDXVKGpuLabelKey] = kDXVKGpuLabel;
  }

  void TearDown() override { string_intern_pool_.clear(); }

  static GpuJob CreateGpuJob(uint64_t timeline_key, uint64_t sw_queue, uint64_t hw_queue,
                             uint64_t hw_execution_begin, uint64_t hw_execution_end) {
    GpuJob gpu_job;
    gpu_job.set_pid(kPid);
    gpu_job.set_tid(kTid);
    gpu_job.set_context(kContext);
    gpu_job.set_seqno(kSeqNo);
    gpu_job.set_timeline_key(timeline_key);
    gpu_job.set_depth(kDepth);
    gpu_job.set_amdgpu_cs_ioctl_time_ns(sw_queue);
    gpu_job.set_amdgpu_sched_run_job_time_ns(hw_queue);
    gpu_job.set_gpu_hardware_start_time_ns(hw_execution_begin);
    gpu_job.set_dma_fence_signaled_time_ns(hw_execution_end);
    return gpu_job;
  }

  static orbit_client_protos::TimerInfo CreateTimerInfo(
      uint64_t start, uint64_t end, int32_t process_id, int32_t processor, int32_t thread_id,
      uint64_t timeline_hash, uint64_t user_data_key, uint32_t depth, uint64_t group_id,
      float alpha, float red, float green, float blue, orbit_client_protos::TimerInfo::Type type) {
    orbit_client_protos::TimerInfo timer;
    timer.set_start(start);
    timer.set_end(end);
    timer.set_process_id(process_id);
    timer.set_thread_id(thread_id);
    timer.set_processor(processor);
    timer.set_timeline_hash(timeline_hash);
    timer.set_user_data_key(user_data_key);
    timer.set_type(type);
    timer.set_depth(depth);
    timer.set_group_id(group_id);
    orbit_client_protos::Color* color = timer.mutable_color();
    color->set_red(static_cast<uint32_t>(red * 255.f));
    color->set_green(static_cast<uint32_t>(green * 255.f));
    color->set_blue(static_cast<uint32_t>(blue * 255.f));
    color->set_alpha(static_cast<uint32_t>(alpha * 255.f));

    return timer;
  }

  static GpuQueueSubmissionMetaInfo* CreateGpuQueueSubmissionMetaInfo(
      GpuQueueSubmission* submission, uint64_t pre_timestamp, uint64_t post_timestamp) {
    GpuQueueSubmissionMetaInfo* meta_info = submission->mutable_meta_info();
    meta_info->set_tid(kTid);
    meta_info->set_pid(kPid);
    meta_info->set_pre_submission_cpu_timestamp(pre_timestamp);
    meta_info->set_post_submission_cpu_timestamp(post_timestamp);
    return meta_info;
  }

  static void AddGpuCommandBufferToGpuSubmitInfo(GpuSubmitInfo* submit_info,
                                                 uint64_t gpu_begin_timestamp,
                                                 uint64_t gpu_end_timestamp) {
    GpuCommandBuffer* command_buffer = submit_info->add_command_buffers();
    command_buffer->set_begin_gpu_timestamp_ns(gpu_begin_timestamp);
    command_buffer->set_end_gpu_timestamp_ns(gpu_end_timestamp);
  }

  static void AddGpuDebugMarkerToGpuQueueSubmission(GpuQueueSubmission* submission,
                                                    GpuQueueSubmissionMetaInfo* begin_meta_info,
                                                    uint64_t marker_text_key,
                                                    uint64_t begin_gpu_timestamp,
                                                    uint64_t end_gpu_timestamp) {
    GpuDebugMarker* debug_marker = submission->add_completed_markers();
    Color* color = debug_marker->mutable_color();
    color->set_alpha(kGpuDebugMarkerAlpha);
    color->set_red(kGpuDebugMarkerRed);
    color->set_green(kGpuDebugMarkerGreen);
    color->set_blue(kGpuDebugMarkerBlue);
    debug_marker->set_depth(kGpuDebugMarkerDepth);
    debug_marker->set_text_key(marker_text_key);
    debug_marker->set_end_gpu_timestamp_ns(end_gpu_timestamp);
    if (begin_meta_info == nullptr) {
      return;
    }
    GpuDebugMarkerBeginInfo* begin_marker = debug_marker->mutable_begin_marker();
    GpuQueueSubmissionMetaInfo* meta_info_copy = begin_marker->mutable_meta_info();
    meta_info_copy->CopyFrom(*begin_meta_info);
    begin_marker->set_gpu_timestamp_ns(begin_gpu_timestamp);
  }

  GpuQueueSubmissionProcessor gpu_queue_submission_processor_;
  static constexpr int32_t kPid = 44;
  static constexpr int32_t kTid = 62;

  static constexpr uint64_t kTimelineKey = 13;
  static constexpr const char* kTimeline = "Timeline";

  static constexpr uint32_t kSeqNo = 112;
  static constexpr uint32_t kContext = 44;
  static constexpr int32_t kDepth = 3;

  static constexpr uint64_t kDXVKGpuLabelKey = 11;
  static constexpr const char* kDXVKGpuLabel = "DXVK__vkFunctionName#123";
  static constexpr uint64_t kDXVKGpuGroupId = 123;
  static constexpr float kGpuDebugMarkerAlpha = 1.f;
  static constexpr float kGpuDebugMarkerRed = 0.75f;
  static constexpr float kGpuDebugMarkerGreen = 0.5f;
  static constexpr float kGpuDebugMarkerBlue = 0.25f;
  static constexpr uint32_t kGpuDebugMarkerDepth = 1;

  absl::flat_hash_map<uint64_t, std::string> string_intern_pool_;
};

TEST_F(GpuQueueSubmissionProcessorTest, DXVKVulkanDebugMarkerEncodesGroupId) {
  orbit_grpc_protos::GpuJob gpu_job = CreateGpuJob(kTimelineKey, 10, 20, 30, 40);

  bool was_called = false;
  static constexpr uint64_t kCommandBufferTextKey = 1234;
  auto get_string_hash_and_send_if_necessary_fake = [&was_called](std::string_view
                                                                  /*str*/) -> uint64_t {
    was_called = true;
    return kCommandBufferTextKey;
  };

  std::vector<orbit_client_protos::TimerInfo> actual_timers =
      gpu_queue_submission_processor_.ProcessGpuJob(gpu_job, string_intern_pool_,
                                                    get_string_hash_and_send_if_necessary_fake);

  EXPECT_FALSE(was_called);
  EXPECT_EQ(actual_timers.size(), 0);

  GpuQueueSubmission submission;
  GpuQueueSubmissionMetaInfo* meta_info = CreateGpuQueueSubmissionMetaInfo(&submission, 9, 11);

  GpuSubmitInfo* submit_info = submission.add_submit_infos();
  AddGpuCommandBufferToGpuSubmitInfo(submit_info, 100, 109);

  AddGpuDebugMarkerToGpuQueueSubmission(&submission, meta_info, kDXVKGpuLabelKey, 101, 108);
  submission.set_num_begin_markers(1);

  actual_timers = gpu_queue_submission_processor_.ProcessGpuQueueSubmission(
      submission, string_intern_pool_, get_string_hash_and_send_if_necessary_fake);

  EXPECT_TRUE(was_called);
  ASSERT_EQ(actual_timers.size(), 2);

  TimerInfo expected_command_buffer_timer =
      CreateTimerInfo(30, 39, kPid, -1, kTid, kTimelineKey, kCommandBufferTextKey, kDepth, 0, 0.f,
                      0.f, 0.f, 0.f, TimerInfo_Type_kGpuCommandBuffer);

  TimerInfo expected_debug_marker = CreateTimerInfo(
      31, 38, kPid, -1, kTid, kTimelineKey, kDXVKGpuLabelKey, kGpuDebugMarkerDepth, kDXVKGpuGroupId,
      kGpuDebugMarkerAlpha, kGpuDebugMarkerRed, kGpuDebugMarkerGreen, kGpuDebugMarkerBlue,
      orbit_client_protos::TimerInfo_Type_kGpuDebugMarker);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_command_buffer_timer, actual_timers[0]));
  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_debug_marker, actual_timers[1]));
}

TEST_F(GpuQueueSubmissionProcessorTest, TryExtractDXVKVulkanGroupIdFromDebugLabel) {
  uint64_t group_id = 0;
  EXPECT_FALSE(GpuQueueSubmissionProcessor::TryExtractDXVKVulkanGroupIdFromDebugLabel(
      "SomeLabelName", &group_id));
  EXPECT_EQ(group_id, 0);

  EXPECT_FALSE(GpuQueueSubmissionProcessor::TryExtractDXVKVulkanGroupIdFromDebugLabel(
      "DXVK__vkFunctionName", &group_id));
  EXPECT_EQ(group_id, 0);

  EXPECT_FALSE(GpuQueueSubmissionProcessor::TryExtractDXVKVulkanGroupIdFromDebugLabel(
      "DXVK__vkFunctionName#abc1", &group_id));
  EXPECT_EQ(group_id, 0);

  EXPECT_TRUE(GpuQueueSubmissionProcessor::TryExtractDXVKVulkanGroupIdFromDebugLabel(
      "DXVK__vkFunctionName#123", &group_id));
  EXPECT_EQ(group_id, 123);

  EXPECT_TRUE(GpuQueueSubmissionProcessor::TryExtractDXVKVulkanGroupIdFromDebugLabel(
      "DXVK__vkFunctionName#456#678", &group_id));
  EXPECT_EQ(group_id, 678);
}

}  // namespace orbit_capture_client