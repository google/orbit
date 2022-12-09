// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <gmock/gmock.h>
#include <grpcpp/channel.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"
#include "SubmissionTracker.h"
#include "VulkanLayerProducer.h"

using ::testing::ElementsAre;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::UnorderedElementsAre;

namespace orbit_vulkan_layer {
namespace {
class MockDispatchTable {
 public:
  MOCK_METHOD(PFN_vkGetQueryPoolResults, GetQueryPoolResults, (VkDevice), ());
  MOCK_METHOD(PFN_vkCmdWriteTimestamp, CmdWriteTimestamp, (VkCommandBuffer), ());
};

PFN_vkCmdWriteTimestamp dummy_write_timestamp_function =
    +[](VkCommandBuffer /*command_buffer*/, VkPipelineStageFlagBits /*pipeline_stage*/,
        VkQueryPool /*query_pool*/, uint32_t /*query*/) {};

class MockTimerQueryPool {
 public:
  MOCK_METHOD(VkQueryPool, GetQueryPool, (VkDevice), ());
  MOCK_METHOD(void, MarkQuerySlotsForReset, (VkDevice, const std::vector<uint32_t>&), ());
  MOCK_METHOD(void, MarkQuerySlotsDoneReading, (VkDevice, const std::vector<uint32_t>&), ());
  MOCK_METHOD(void, RollbackPendingQuerySlots, (VkDevice, const std::vector<uint32_t>&), ());
  MOCK_METHOD(bool, NextReadyQuerySlot, (VkDevice, uint32_t*), ());
  MOCK_METHOD(void, PrintStats, (), ());
};

class MockDeviceManager {
 public:
  MOCK_METHOD(VkPhysicalDevice, GetPhysicalDeviceOfLogicalDevice, (VkDevice), ());
  MOCK_METHOD(VkPhysicalDeviceProperties, GetPhysicalDeviceProperties, (VkPhysicalDevice), ());
};

class MockVulkanLayerProducer : public VulkanLayerProducer {
 public:
  MOCK_METHOD(bool, IsCapturing, (), (override));
  MOCK_METHOD(uint64_t, InternStringIfNecessaryAndGetKey, (std::string), (override));
  MOCK_METHOD(bool, EnqueueCaptureEvent, (orbit_grpc_protos::ProducerCaptureEvent && capture_event),
              (override));

  MOCK_METHOD(void, BringUp, (const std::shared_ptr<grpc::Channel>& channel), (override));
  MOCK_METHOD(void, TakeDown, (), (override));

  MOCK_METHOD(void, SetCaptureStatusListener, (CaptureStatusListener*), (override));

  void StartCapture(
      uint64_t max_local_marker_depth_per_command_buffer = std::numeric_limits<uint64_t>::max()) {
    is_capturing_ = true;
    ASSERT_NE(listener_, nullptr);
    orbit_grpc_protos::CaptureOptions capture_options{};
    capture_options.set_max_local_marker_depth_per_command_buffer(
        max_local_marker_depth_per_command_buffer);
    listener_->OnCaptureStart(capture_options);
  }

  void StopCapture() {
    is_capturing_ = false;
    ASSERT_NE(listener_, nullptr);
    listener_->OnCaptureStop();
    listener_->OnCaptureFinished();
  }

  void FakeSetCaptureStatusListener(VulkanLayerProducer::CaptureStatusListener* listener) {
    listener_ = listener;
  }

  bool is_capturing_ = false;

 private:
  VulkanLayerProducer::CaptureStatusListener* listener_ = nullptr;
};

using Color = SubmissionTracker<MockDispatchTable, MockDeviceManager, MockTimerQueryPool>::Color;
using QueueSubmission =
    SubmissionTracker<MockDispatchTable, MockDeviceManager, MockTimerQueryPool>::QueueSubmission;

}  // namespace

class SubmissionTrackerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    producer_ = std::make_unique<MockVulkanLayerProducer>();
    auto set_capture_status_listener_function =
        [this](VulkanLayerProducer::CaptureStatusListener* listener) {
          producer_->FakeSetCaptureStatusListener(listener);
        };
    EXPECT_CALL(*producer_, SetCaptureStatusListener)
        .Times(1)
        .WillOnce(Invoke(set_capture_status_listener_function));
    tracker_.SetVulkanLayerProducer(producer_.get());
    auto is_capturing_function = [this]() -> bool { return producer_->is_capturing_; };
    EXPECT_CALL(*producer_, IsCapturing).WillRepeatedly(Invoke(is_capturing_function));
    EXPECT_CALL(timer_query_pool_, GetQueryPool).WillRepeatedly(Return(query_pool_));
    EXPECT_CALL(device_manager_, GetPhysicalDeviceOfLogicalDevice)
        .WillRepeatedly(Return(physical_device_));
    EXPECT_CALL(device_manager_, GetPhysicalDeviceProperties)
        .WillRepeatedly(Return(physical_device_properties_));

    EXPECT_CALL(dispatch_table_, CmdWriteTimestamp)
        .WillRepeatedly(Return(dummy_write_timestamp_function));
  }

  void TearDown() override {
    EXPECT_CALL(*producer_, SetCaptureStatusListener(nullptr)).Times(1);
    tracker_.SetVulkanLayerProducer(nullptr);
    producer_.reset();
  }

  MockDispatchTable dispatch_table_;
  MockTimerQueryPool timer_query_pool_;
  MockDeviceManager device_manager_;
  std::unique_ptr<MockVulkanLayerProducer> producer_;
  SubmissionTracker<MockDispatchTable, MockDeviceManager, MockTimerQueryPool> tracker_ =
      SubmissionTracker<MockDispatchTable, MockDeviceManager, MockTimerQueryPool>(
          &dispatch_table_, &timer_query_pool_, &device_manager_,
          std::numeric_limits<uint32_t>::max());

  VkDevice device_ = {};
  VkCommandPool command_pool_ = {};
  VkCommandBuffer command_buffer_ = {};
  VkQueryPool query_pool_ = {};
  VkPhysicalDevice physical_device_ = {};
  VkPhysicalDeviceProperties physical_device_properties_ = {.limits = {.timestampPeriod = 1.f}};
  VkQueue queue_ = {};
  VkSubmitInfo submit_info_ = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                               .pNext = nullptr,
                               .commandBufferCount = 1,
                               .pCommandBuffers = &command_buffer_};

  static constexpr uint32_t kSlotIndex1 = 32;
  static constexpr uint32_t kSlotIndex2 = 33;
  static constexpr uint32_t kSlotIndex3 = 34;
  static constexpr uint32_t kSlotIndex4 = 35;
  static constexpr uint32_t kSlotIndex5 = 36;
  static constexpr uint32_t kSlotIndex6 = 37;
  static constexpr uint32_t kSlotIndex7 = 38;

  static bool MockNextReadyQuerySlot1(VkDevice /*device*/, uint32_t* allocated_slot) {
    *allocated_slot = kSlotIndex1;
    return true;
  }

  static bool MockNextReadyQuerySlot2(VkDevice /*device*/, uint32_t* allocated_slot) {
    *allocated_slot = kSlotIndex2;
    return true;
  }

  static bool MockNextReadyQuerySlot3(VkDevice /*device*/, uint32_t* allocated_slot) {
    *allocated_slot = kSlotIndex3;
    return true;
  }

  static bool MockNextReadyQuerySlot4(VkDevice /*device*/, uint32_t* allocated_slot) {
    *allocated_slot = kSlotIndex4;
    return true;
  }

  static bool MockNextReadyQuerySlot5(VkDevice /*device*/, uint32_t* allocated_slot) {
    *allocated_slot = kSlotIndex5;
    return true;
  }

  static bool MockNextReadyQuerySlot6(VkDevice /*device*/, uint32_t* allocated_slot) {
    *allocated_slot = kSlotIndex6;
    return true;
  }

  static bool MockNextReadyQuerySlot7(VkDevice /*device*/, uint32_t* allocated_slot) {
    *allocated_slot = kSlotIndex7;
    return true;
  }

  static constexpr uint64_t kTimestamp1 = 11;
  static constexpr uint64_t kTimestamp2 = 12;
  static constexpr uint64_t kTimestamp3 = 13;
  static constexpr uint64_t kTimestamp4 = 14;
  static constexpr uint64_t kTimestamp5 = 15;
  static constexpr uint64_t kTimestamp6 = 16;
  static constexpr uint64_t kTimestamp7 = 17;

  const PFN_vkGetQueryPoolResults mock_get_query_pool_results_function_all_ready_ =
      +[](VkDevice /*device*/, VkQueryPool /*queryPool*/, uint32_t first_query,
          uint32_t query_count, size_t /*dataSize*/, void* data, VkDeviceSize /*stride*/,
          VkQueryResultFlags flags) -> VkResult {
    EXPECT_EQ(query_count, 1);
    EXPECT_NE((flags & VK_QUERY_RESULT_64_BIT), 0);
    switch (first_query) {
      case kSlotIndex1:
        *absl::bit_cast<uint64_t*>(data) = kTimestamp1;
        break;
      case kSlotIndex2:
        *absl::bit_cast<uint64_t*>(data) = kTimestamp2;
        break;
      case kSlotIndex3:
        *absl::bit_cast<uint64_t*>(data) = kTimestamp3;
        break;
      case kSlotIndex4:
        *absl::bit_cast<uint64_t*>(data) = kTimestamp4;
        break;
      case kSlotIndex5:
        *absl::bit_cast<uint64_t*>(data) = kTimestamp5;
        break;
      case kSlotIndex6:
        *absl::bit_cast<uint64_t*>(data) = kTimestamp6;
        break;
      case kSlotIndex7:
        *absl::bit_cast<uint64_t*>(data) = kTimestamp7;
        break;
      default:
        ORBIT_UNREACHABLE();
    }
    return VK_SUCCESS;
  };

  const PFN_vkGetQueryPoolResults mock_get_query_pool_results_function_not_ready_ =
      +[](VkDevice /*device*/, VkQueryPool /*queryPool*/, uint32_t /*first_query*/,
          uint32_t /*query_count*/, size_t /*dataSize*/, void* /*data*/, VkDeviceSize /*stride*/,
          VkQueryResultFlags /*flags*/) -> VkResult { return VK_NOT_READY; };

  static void ExpectSingleCommandBufferSubmissionEq(
      const orbit_grpc_protos::ProducerCaptureEvent& actual_capture_event,
      uint64_t test_pre_submit_time, uint64_t test_post_submit_time, pid_t expected_tid,
      pid_t expected_pid, uint64_t expected_command_buffer_begin_timestamp,
      uint64_t expected_command_buffer_end_timestamp) {
    EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
    const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
        actual_capture_event.gpu_queue_submission();

    ExpectSubmitEq(actual_queue_submission.meta_info(), test_pre_submit_time, test_post_submit_time,
                   expected_tid, expected_pid);

    ASSERT_EQ(actual_queue_submission.submit_infos_size(), 1);
    const orbit_grpc_protos::GpuSubmitInfo& actual_submit_info =
        actual_queue_submission.submit_infos(0);

    ASSERT_EQ(actual_submit_info.command_buffers_size(), 1);
    const orbit_grpc_protos::GpuCommandBuffer& actual_command_buffer =
        actual_submit_info.command_buffers(0);

    EXPECT_EQ(expected_command_buffer_begin_timestamp,
              actual_command_buffer.begin_gpu_timestamp_ns());
    EXPECT_EQ(expected_command_buffer_end_timestamp, actual_command_buffer.end_gpu_timestamp_ns());
  }

  static void ExpectSubmitEq(const orbit_grpc_protos::GpuQueueSubmissionMetaInfo& actual_meta_info,
                             int64_t test_pre_submit_time, uint64_t test_post_submit_time,
                             pid_t expected_tid, pid_t expected_pid) {
    EXPECT_LE(test_pre_submit_time, actual_meta_info.pre_submission_cpu_timestamp());
    EXPECT_LE(actual_meta_info.pre_submission_cpu_timestamp(),
              actual_meta_info.post_submission_cpu_timestamp());
    EXPECT_LE(actual_meta_info.post_submission_cpu_timestamp(), test_post_submit_time);
    EXPECT_EQ(expected_tid, actual_meta_info.tid());
    EXPECT_EQ(expected_pid, actual_meta_info.pid());
  }

  static void ExpectDebugMarkerEndEq(const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker,
                                     uint64_t expected_end_timestamp, uint64_t expected_text_key,
                                     Color expected_color, int32_t expected_depth) {
    EXPECT_EQ(actual_debug_marker.end_gpu_timestamp_ns(), expected_end_timestamp);
    EXPECT_EQ(actual_debug_marker.color().red(), expected_color.red);
    EXPECT_EQ(actual_debug_marker.color().green(), expected_color.green);
    EXPECT_EQ(actual_debug_marker.color().blue(), expected_color.blue);
    EXPECT_EQ(actual_debug_marker.color().alpha(), expected_color.alpha);
    EXPECT_EQ(actual_debug_marker.text_key(), expected_text_key);
    EXPECT_EQ(actual_debug_marker.depth(), expected_depth);
  }

  static void ExpectDebugMarkerBeginEq(const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker,
                                       uint64_t expected_timestamp, int64_t test_pre_submit_time,
                                       uint64_t test_post_submit_time, pid_t expected_tid,
                                       pid_t expected_pid) {
    ASSERT_TRUE(actual_debug_marker.has_begin_marker());
    EXPECT_EQ(actual_debug_marker.begin_marker().gpu_timestamp_ns(), expected_timestamp);
    const orbit_grpc_protos::GpuQueueSubmissionMetaInfo& actual_begin_marker_meta_info =
        actual_debug_marker.begin_marker().meta_info();
    ExpectSubmitEq(actual_begin_marker_meta_info, test_pre_submit_time, test_post_submit_time,
                   expected_tid, expected_pid);
  }

  void ExpectTwoNextReadyQuerySlotCalls() {
    EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
        .Times(2)
        .WillOnce(Invoke(MockNextReadyQuerySlot1))
        .WillOnce(Invoke(MockNextReadyQuerySlot2));
  }

  void ExpectThreeNextReadyQuerySlotCalls() {
    EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
        .Times(3)
        .WillOnce(Invoke(MockNextReadyQuerySlot1))
        .WillOnce(Invoke(MockNextReadyQuerySlot2))
        .WillOnce(Invoke(MockNextReadyQuerySlot3));
  }

  void ExpectFourNextReadyQuerySlotCalls() {
    EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
        .Times(4)
        .WillOnce(Invoke(MockNextReadyQuerySlot1))
        .WillOnce(Invoke(MockNextReadyQuerySlot2))
        .WillOnce(Invoke(MockNextReadyQuerySlot3))
        .WillOnce(Invoke(MockNextReadyQuerySlot4));
  }

  void ExpectSixNextReadyQuerySlotCalls() {
    EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
        .Times(6)
        .WillOnce(Invoke(MockNextReadyQuerySlot1))
        .WillOnce(Invoke(MockNextReadyQuerySlot2))
        .WillOnce(Invoke(MockNextReadyQuerySlot3))
        .WillOnce(Invoke(MockNextReadyQuerySlot4))
        .WillOnce(Invoke(MockNextReadyQuerySlot5))
        .WillOnce(Invoke(MockNextReadyQuerySlot6));
  }

  void ExpectSevenNextReadyQuerySlotCalls() {
    EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
        .Times(7)
        .WillOnce(Invoke(MockNextReadyQuerySlot1))
        .WillOnce(Invoke(MockNextReadyQuerySlot2))
        .WillOnce(Invoke(MockNextReadyQuerySlot3))
        .WillOnce(Invoke(MockNextReadyQuerySlot4))
        .WillOnce(Invoke(MockNextReadyQuerySlot5))
        .WillOnce(Invoke(MockNextReadyQuerySlot6))
        .WillOnce(Invoke(MockNextReadyQuerySlot7));
  }
};

TEST(SubmissionTracker, CanBeInitialized) {
  MockDispatchTable dispatch_table;
  MockTimerQueryPool timer_query_pool;
  MockDeviceManager device_manager;
  SubmissionTracker<MockDispatchTable, MockDeviceManager, MockTimerQueryPool> tracker(
      &dispatch_table, &timer_query_pool, &device_manager, std::numeric_limits<uint32_t>::max());
}

TEST(SubmissionTracker, SetVulkanLayerProducerWillCallSetListener) {
  MockDispatchTable dispatch_table;
  MockTimerQueryPool timer_query_pool;
  MockDeviceManager device_manager;
  std::unique_ptr<MockVulkanLayerProducer> producer = std::make_unique<MockVulkanLayerProducer>();

  SubmissionTracker<MockDispatchTable, MockDeviceManager, MockTimerQueryPool> tracker(
      &dispatch_table, &timer_query_pool, &device_manager, std::numeric_limits<uint32_t>::max());

  VulkanLayerProducer::CaptureStatusListener* actual_listener;
  EXPECT_CALL(*producer, SetCaptureStatusListener).Times(1).WillOnce(SaveArg<0>(&actual_listener));
  tracker.SetVulkanLayerProducer(producer.get());
  EXPECT_EQ(actual_listener, &tracker);
}

TEST_F(SubmissionTrackerTest, CannotUntrackAnUntrackedCommandBuffer) {
  EXPECT_DEATH({ tracker_.UntrackCommandBuffers(device_, command_pool_, &command_buffer_, 1); },
               "");
}

TEST_F(SubmissionTrackerTest, CanTrackCommandBufferAgainAfterUntrack) {
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.UntrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
}

TEST_F(SubmissionTrackerTest, UntrackingACommandBufferWillRollbackTheSlots) {
  std::vector<uint32_t> actual_slots_to_rollback;
  EXPECT_CALL(timer_query_pool_, RollbackPendingQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_to_rollback));

  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(1)
      .WillOnce(Invoke(MockNextReadyQuerySlot1));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.UntrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  producer_->StopCapture();

  EXPECT_THAT(actual_slots_to_rollback, ElementsAre(kSlotIndex1));
}

TEST_F(SubmissionTrackerTest, MarkCommandBufferBeginWontWriteTimestampsWhenNotCapturing) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot).Times(0);

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
}

TEST_F(SubmissionTrackerTest, MarkCommandBufferBeginWillWriteTimestampWhenCapturing) {
  // We cannot capture anything in that lambda as we need to cast it to a function pointer.
  static bool was_called = false;

  PFN_vkCmdWriteTimestamp mock_write_timestamp_function =
      +[](VkCommandBuffer /*command_buffer*/, VkPipelineStageFlagBits /*pipeline_stage*/,
          VkQueryPool /*query_pool*/, uint32_t query) {
        EXPECT_EQ(query, kSlotIndex1);
        was_called = true;
      };

  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(1)
      .WillOnce(Invoke(MockNextReadyQuerySlot1));
  EXPECT_CALL(dispatch_table_, CmdWriteTimestamp)
      .Times(1)
      .WillOnce(Return(mock_write_timestamp_function));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);

  EXPECT_TRUE(was_called);
  was_called = false;
}

TEST_F(SubmissionTrackerTest, ResetCommandBufferShouldRollbackUnsubmittedSlots) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(1)
      .WillOnce(Invoke(MockNextReadyQuerySlot1));
  std::vector<uint32_t> actual_slots_to_rollback;
  EXPECT_CALL(timer_query_pool_, RollbackPendingQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_to_rollback));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.ResetCommandBuffer(command_buffer_);

  EXPECT_THAT(actual_slots_to_rollback, ElementsAre(kSlotIndex1));
}

TEST_F(SubmissionTrackerTest, ResetCommandPoolShouldRollbackUnsubmittedSlots) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(1)
      .WillOnce(Invoke(MockNextReadyQuerySlot1));
  std::vector<uint32_t> actual_slots_to_rollback;
  EXPECT_CALL(timer_query_pool_, RollbackPendingQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_to_rollback));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.ResetCommandPool(command_pool_);

  EXPECT_THAT(actual_slots_to_rollback, ElementsAre(kSlotIndex1));
}

TEST_F(SubmissionTrackerTest, MarkCommandBufferEndWontWriteTimestampsWhenNotCapturing) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot).Times(0);
  EXPECT_CALL(dispatch_table_, CmdWriteTimestamp).Times(0);

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
}

TEST_F(SubmissionTrackerTest, MarkCommandBufferEndWillWriteTimestampsWhenBeginNotCaptured) {
  static bool was_called = false;

  PFN_vkCmdWriteTimestamp mock_write_timestamp_function =
      +[](VkCommandBuffer /*command_buffer*/, VkPipelineStageFlagBits /*pipeline_stage*/,
          VkQueryPool /*query_pool*/, uint32_t query) {
        EXPECT_EQ(query, kSlotIndex1);
        was_called = true;
      };

  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(1)
      .WillOnce(Invoke(MockNextReadyQuerySlot1));
  EXPECT_CALL(dispatch_table_, CmdWriteTimestamp)
      .Times(1)
      .WillOnce(Return(mock_write_timestamp_function));

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  producer_->StartCapture();
  tracker_.MarkCommandBufferEnd(command_buffer_);

  EXPECT_TRUE(was_called);
  was_called = false;
}

TEST_F(SubmissionTrackerTest, CanRetrieveCommandBufferTimestampsForACompleteSubmission) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_marked_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_marked_done_reading));
  orbit_grpc_protos::ProducerCaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(1)
      .WillOnce(Invoke(mock_enqueue_capture_event));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint64_t pre_submit_time = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = orbit_base::CaptureTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_marked_done_reading, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
  ExpectSingleCommandBufferSubmissionEq(actual_capture_event, pre_submit_time, post_submit_time,
                                        tid, pid, kTimestamp1, kTimestamp2);
}

TEST_F(SubmissionTrackerTest,
       CanRetrieveCommandBufferTimestampsForACompleteSubmissionAtALaterTime) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillOnce(Return(mock_get_query_pool_results_function_not_ready_))
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading));
  orbit_grpc_protos::ProducerCaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(1)
      .WillOnce(Invoke(mock_enqueue_capture_event));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint64_t pre_submit_time = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = orbit_base::CaptureTimestampNs();
  tracker_.CompleteSubmits(device_);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
  ExpectSingleCommandBufferSubmissionEq(actual_capture_event, pre_submit_time, post_submit_time,
                                        tid, pid, kTimestamp1, kTimestamp2);
}

TEST_F(SubmissionTrackerTest, WillRetryCompletingSubmissionsWhenTimestampQueryFails) {
  // We are persisting two submits with two different command buffers for completion, where
  // on the second submit one of the timestamps query calls is intended to fail for the
  // purpose of the test. This submit must be tried again and successfully completed on the
  // second attempt.

  ExpectFourNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      // The first two calls should succeed to complete the first submission.
      .WillOnce(Return(mock_get_query_pool_results_function_all_ready_))
      .WillOnce(Return(mock_get_query_pool_results_function_all_ready_))
      // Fail on the second submission so that we retry on the second call.
      .WillOnce(Return(mock_get_query_pool_results_function_all_ready_))
      .WillOnce(Return(mock_get_query_pool_results_function_not_ready_))
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));

  std::vector<uint32_t> actual_slots_done_reading1;
  std::vector<uint32_t> actual_slots_done_reading2;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(2)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading1))
      .WillOnce(SaveArg<1>(&actual_slots_done_reading2));

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> actual_capture_events;
  auto mock_enqueue_capture_event =
      [&actual_capture_events](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_events.push_back(std::move(capture_event));
        return true;
      };
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(2)
      .WillRepeatedly(Invoke(mock_enqueue_capture_event));

  producer_->StartCapture();

  // VkCommandBuffer's are opaque handles and we can't allocate them properly as we can't create
  // a device in the context of the text. Using absl::bit_cast is a workaround to get two
  // VkCommandBuffer's that are different, which is needed for this test.
  std::array<VkCommandBuffer, 2> command_buffers{absl::bit_cast<VkCommandBuffer>(1L),
                                                 absl::bit_cast<VkCommandBuffer>(2L)};
  ORBIT_CHECK(command_buffers[0] != command_buffers[1]);

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffers[0], 2);
  tracker_.MarkCommandBufferBegin(command_buffers[0]);
  tracker_.MarkCommandBufferEnd(command_buffers[0]);
  tracker_.MarkCommandBufferBegin(command_buffers[1]);
  tracker_.MarkCommandBufferEnd(command_buffers[1]);

  std::array<VkSubmitInfo, 2> submit_infos{VkSubmitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                                        .pNext = nullptr,
                                                        .commandBufferCount = 1,
                                                        .pCommandBuffers = &command_buffers[0]},
                                           VkSubmitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                                        .pNext = nullptr,
                                                        .commandBufferCount = 1,
                                                        .pCommandBuffers = &command_buffers[1]}};

  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();
  std::array<uint64_t, 2> pre_submit_times{};
  std::array<uint64_t, 2> post_submit_times{};

  pre_submit_times[0] = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_infos[0]);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_infos[0], queue_submission_optional);
  post_submit_times[0] = orbit_base::CaptureTimestampNs();

  pre_submit_times[1] = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional2 =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_infos[1]);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_infos[1], queue_submission_optional2);
  post_submit_times[1] = orbit_base::CaptureTimestampNs();

  tracker_.CompleteSubmits(device_);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading1,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex4));
  // We failed on the 4th GetQuerySlots call above, which belongs to the "begin" timestamp of the
  // second command buffer (note that the order of calls when querying for timestamps is to query
  // for the end timestamp first). Therefore, this must be kSlotIndex3.
  EXPECT_THAT(actual_slots_done_reading2, UnorderedElementsAre(kSlotIndex3));

  ExpectSingleCommandBufferSubmissionEq(actual_capture_events[0], pre_submit_times[0],
                                        post_submit_times[0], tid, pid, kTimestamp1, kTimestamp2);
  ExpectSingleCommandBufferSubmissionEq(actual_capture_events[1], pre_submit_times[1],
                                        post_submit_times[1], tid, pid, kTimestamp3, kTimestamp4);
}

TEST_F(SubmissionTrackerTest, StopCaptureBeforeSubmissionWillResetTheSlots) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults).Times(0);
  std::vector<uint32_t> actual_slots_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading));

  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(0);

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  producer_->StopCapture();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
}

TEST_F(SubmissionTrackerTest,
       CommandBufferTimestampsRecordedWhenCapturingCanBeRetrievedWhenNotCapturing) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading));
  orbit_grpc_protos::ProducerCaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(1)
      .WillOnce(Invoke(mock_enqueue_capture_event));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint64_t pre_submit_time = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = orbit_base::CaptureTimestampNs();
  producer_->StopCapture();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
  ExpectSingleCommandBufferSubmissionEq(actual_capture_event, pre_submit_time, post_submit_time,
                                        tid, pid, kTimestamp1, kTimestamp2);
}

TEST_F(SubmissionTrackerTest, StopCaptureDuringSubmissionWillStillYieldResults) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading));
  orbit_grpc_protos::ProducerCaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(1)
      .WillOnce(Invoke(mock_enqueue_capture_event));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint64_t pre_submit_time = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  producer_->StopCapture();
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = orbit_base::CaptureTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
  ExpectSingleCommandBufferSubmissionEq(actual_capture_event, pre_submit_time, post_submit_time,
                                        tid, pid, kTimestamp1, kTimestamp2);
}

TEST_F(SubmissionTrackerTest, StartCaptureJustBeforeSubmissionWontWriteData) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot).Times(0);
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults).Times(0);
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading).Times(0);
  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(0);

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  producer_->StartCapture();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);
}

TEST_F(SubmissionTrackerTest, StartCaptureDuringSubmissionWontWriteData) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot).Times(0);
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults).Times(0);
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading).Times(0);
  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(0);

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  producer_->StartCapture();
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);
}

TEST_F(SubmissionTrackerTest, MultipleSubmissionsWontBeOutOfOrder) {
  ExpectFourNextReadyQuerySlotCalls();
  // Ensure that the first submission will be considered to be not ready after the first present.
  // Afterwards, all submissions are considered ready.
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillOnce(Return(mock_get_query_pool_results_function_not_ready_))
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));

  std::vector<uint32_t> actual_slots_done_reading;
  auto fake_mark_query_slots_done_reading = [&actual_slots_done_reading](
                                                VkDevice /*device*/,
                                                const std::vector<uint32_t>& slots_to_reset) {
    actual_slots_done_reading.insert(actual_slots_done_reading.end(), slots_to_reset.begin(),
                                     slots_to_reset.end());
  };
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .WillRepeatedly(Invoke(fake_mark_query_slots_done_reading));

  std::vector<uint32_t> actual_slots_marked_reset;
  auto fake_mark_query_slots_for_reset = [&actual_slots_marked_reset](
                                             VkDevice /*device*/,
                                             const std::vector<uint32_t>& slots_to_reset) {
    actual_slots_marked_reset.insert(actual_slots_marked_reset.end(), slots_to_reset.begin(),
                                     slots_to_reset.end());
  };
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsForReset)
      .WillRepeatedly(Invoke(fake_mark_query_slots_for_reset));

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> actual_capture_events;
  auto fake_enqueue_capture_event =
      [&actual_capture_events](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_events.emplace_back(std::move(capture_event));
        return true;
      };
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(2)
      .WillRepeatedly(Invoke(fake_enqueue_capture_event));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);

  // Submission #1:
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional_1 =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_1);
  // The results are not yet ready at this point in time.
  tracker_.CompleteSubmits(device_);

  // Submission #2:
  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional_2 =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_2);
  // Now the results are ready for all submissions.
  tracker_.CompleteSubmits(device_);
  tracker_.ResetCommandBuffer(command_buffer_);

  ASSERT_EQ(actual_capture_events.size(), 2);
  EXPECT_TRUE(actual_capture_events[0].has_gpu_queue_submission());
  EXPECT_TRUE(actual_capture_events[1].has_gpu_queue_submission());
  EXPECT_LT(actual_capture_events[0]
                .mutable_gpu_queue_submission()
                ->meta_info()
                .pre_submission_cpu_timestamp(),
            actual_capture_events[1]
                .mutable_gpu_queue_submission()
                ->meta_info()
                .pre_submission_cpu_timestamp());
  EXPECT_THAT(actual_slots_done_reading,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4));
  EXPECT_THAT(actual_slots_marked_reset,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4));
}

TEST_F(SubmissionTrackerTest, WillResetProperlyWhenStartStopAndStartACaptureWithinASubmission) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(1)
      .WillOnce(Invoke(MockNextReadyQuerySlot1));
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults).Times(0);
  std::vector<uint32_t> actual_slots_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(0);

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  producer_->StartCapture();
  tracker_.MarkCommandBufferBegin(command_buffer_);
  producer_->StopCapture();
  tracker_.MarkCommandBufferEnd(command_buffer_);
  producer_->StartCapture();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading, UnorderedElementsAre(kSlotIndex1));
}

TEST_F(SubmissionTrackerTest, ReusingCommandBufferWithoutResetInvalidatesSlot) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_marked_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_marked_done_reading));
  orbit_grpc_protos::ProducerCaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(1)
      .WillOnce(Invoke(mock_enqueue_capture_event));
  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint64_t pre_submit_time = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = orbit_base::CaptureTimestampNs();
  tracker_.CompleteSubmits(device_);

  ExpectSingleCommandBufferSubmissionEq(actual_capture_event, pre_submit_time, post_submit_time,
                                        tid, pid, kTimestamp1, kTimestamp2);

  EXPECT_THAT(actual_slots_marked_done_reading, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));

  queue_submission_optional = tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);
}

TEST_F(SubmissionTrackerTest, CanReuseCommandBufferAfterReset) {
  ExpectThreeNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading).Times(1);
  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(1);

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);
  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkCommandBufferBegin(command_buffer_);
}

TEST_F(SubmissionTrackerTest, DebugMarkerBeginWillWriteTimestampWhenCapturing) {
  // We cannot capture anything in that lambda as we need to cast it to a function pointer.
  static bool was_called = false;

  PFN_vkCmdWriteTimestamp mock_write_timestamp_function =
      +[](VkCommandBuffer /*command_buffer*/, VkPipelineStageFlagBits /*pipeline_stage*/,
          VkQueryPool /*query_pool*/, uint32_t query) {
        EXPECT_EQ(query, kSlotIndex2);
        was_called = true;
      };

  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, CmdWriteTimestamp)
      .Times(2)
      .WillOnce(Return(dummy_write_timestamp_function))
      .WillOnce(Return(mock_write_timestamp_function));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, "Marker", {});

  EXPECT_TRUE(was_called);
  was_called = false;
}

TEST_F(SubmissionTrackerTest, ResetCommandBufferShouldRollbackUnsubmittedMarkerSlots) {
  ExpectTwoNextReadyQuerySlotCalls();
  std::vector<uint32_t> actual_slots_to_rollback;
  EXPECT_CALL(timer_query_pool_, RollbackPendingQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_to_rollback));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, "Marker", {});
  tracker_.ResetCommandBuffer(command_buffer_);

  EXPECT_THAT(actual_slots_to_rollback, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
}

TEST_F(SubmissionTrackerTest, DebugMarkerBeginWontWriteTimestampsWhenNotCapturing) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot).Times(0);
  EXPECT_CALL(dispatch_table_, CmdWriteTimestamp).Times(0);

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, "Marker", {});
}

TEST_F(SubmissionTrackerTest, DebugMarkerEndWontWriteTimestampsWhenNotCapturing) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot).Times(0);
  EXPECT_CALL(dispatch_table_, CmdWriteTimestamp).Times(0);

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, "Marker", {});
  tracker_.MarkDebugMarkerEnd(command_buffer_);
}

TEST_F(SubmissionTrackerTest, CanRetrieveDebugMarkerTimestampsForACompleteSubmission) {
  ExpectFourNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading));
  orbit_grpc_protos::ProducerCaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };

  const char* text = "Text";
  constexpr uint64_t kExpectedTextKey = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text](const std::string& str) {
    EXPECT_STREQ(text, str.c_str());
    return kExpectedTextKey;
  };
  EXPECT_CALL(*producer_, InternStringIfNecessaryAndGetKey)
      .Times(1)
      .WillOnce(Invoke(mock_intern_string_if_necessary_and_get_key));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(1)
      .WillOnce(Invoke(mock_enqueue_capture_event));

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text, expected_color);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint64_t pre_submit_time = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = orbit_base::CaptureTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4));
  EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
      actual_capture_event.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission.num_begin_markers(), 1);
  EXPECT_EQ(actual_queue_submission.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker =
      actual_queue_submission.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker, kTimestamp3, kExpectedTextKey, expected_color, 0);
  ExpectDebugMarkerBeginEq(actual_debug_marker, kTimestamp2, pre_submit_time, post_submit_time, tid,
                           pid);
}

TEST_F(SubmissionTrackerTest, CanRetrieveDebugMarkerEndEvenWhenBeginNotCaptured) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading));
  orbit_grpc_protos::ProducerCaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };

  const char* text = "Text";
  constexpr uint64_t kExpectedTextKey = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text](const std::string& str) {
    EXPECT_STREQ(text, str.c_str());
    return kExpectedTextKey;
  };
  EXPECT_CALL(*producer_, InternStringIfNecessaryAndGetKey)
      .Times(1)
      .WillOnce(Invoke(mock_intern_string_if_necessary_and_get_key));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(1)
      .WillOnce(Invoke(mock_enqueue_capture_event));

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text, expected_color);
  producer_->StartCapture();
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
  EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
      actual_capture_event.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission.num_begin_markers(), 0);
  EXPECT_EQ(actual_queue_submission.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker =
      actual_queue_submission.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker, kTimestamp1, kExpectedTextKey, expected_color, 0);
  EXPECT_FALSE(actual_debug_marker.has_begin_marker());
}

TEST_F(SubmissionTrackerTest, CanRetrieveNestedDebugMarkerTimestampsForACompleteSubmission) {
  ExpectSixNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading));
  orbit_grpc_protos::ProducerCaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };

  const std::string text_outer = "Outer";
  const std::string text_inner = "Inner";
  constexpr uint64_t kExpectedTextKeyOuter = 111;
  constexpr uint64_t kExpectedTextKeyInner = 112;
  auto mock_intern_string_if_necessary_and_get_key = [&text_outer,
                                                      &text_inner](const std::string& str) {
    if (str == text_outer) {
      return kExpectedTextKeyOuter;
    }
    if (str == text_inner) {
      return kExpectedTextKeyInner;
    }
    ORBIT_UNREACHABLE();
  };
  EXPECT_CALL(*producer_, InternStringIfNecessaryAndGetKey)
      .Times(2)
      .WillRepeatedly(Invoke(mock_intern_string_if_necessary_and_get_key));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(1)
      .WillOnce(Invoke(mock_enqueue_capture_event));

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_outer.c_str(), expected_color);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_inner.c_str(), expected_color);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint64_t pre_submit_time = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = orbit_base::CaptureTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4, kSlotIndex5,
                                   kSlotIndex6));
  EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
      actual_capture_event.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission.num_begin_markers(), 2);
  EXPECT_EQ(actual_queue_submission.completed_markers_size(), 2);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker_inner =
      actual_queue_submission.completed_markers(0);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker_outer =
      actual_queue_submission.completed_markers(1);

  ExpectDebugMarkerEndEq(actual_debug_marker_outer, kTimestamp5, kExpectedTextKeyOuter,
                         expected_color, 0);
  ExpectDebugMarkerBeginEq(actual_debug_marker_outer, kTimestamp2, pre_submit_time,
                           post_submit_time, tid, pid);

  ExpectDebugMarkerEndEq(actual_debug_marker_inner, kTimestamp4, kExpectedTextKeyInner,
                         expected_color, 1);
  ExpectDebugMarkerBeginEq(actual_debug_marker_inner, kTimestamp3, pre_submit_time,
                           post_submit_time, tid, pid);
}

TEST_F(SubmissionTrackerTest,
       CanRetrieveNextedDebugMarkerTimestampsForASubmissionMissingFirstBegin) {
  ExpectFourNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading));
  orbit_grpc_protos::ProducerCaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };

  const std::string text_outer = "Outer";
  const std::string text_inner = "Inner";
  constexpr uint64_t kExpectedTextKeyOuter = 111;
  constexpr uint64_t kExpectedTextKeyInner = 112;
  auto mock_intern_string_if_necessary_and_get_key = [&text_outer,
                                                      &text_inner](const std::string& str) {
    if (str == text_outer) {
      return kExpectedTextKeyOuter;
    }
    if (str == text_inner) {
      return kExpectedTextKeyInner;
    }
    ORBIT_UNREACHABLE();
  };
  EXPECT_CALL(*producer_, InternStringIfNecessaryAndGetKey)
      .Times(2)
      .WillRepeatedly(Invoke(mock_intern_string_if_necessary_and_get_key));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(1)
      .WillOnce(Invoke(mock_enqueue_capture_event));

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_outer.c_str(), expected_color);
  producer_->StartCapture();
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_inner.c_str(), expected_color);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint64_t pre_submit_time = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = orbit_base::CaptureTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4));
  EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
      actual_capture_event.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission.num_begin_markers(), 1);
  EXPECT_EQ(actual_queue_submission.completed_markers_size(), 2);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker_inner =
      actual_queue_submission.completed_markers(0);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker_outer =
      actual_queue_submission.completed_markers(1);

  ExpectDebugMarkerEndEq(actual_debug_marker_outer, kTimestamp3, kExpectedTextKeyOuter,
                         expected_color, 0);
  EXPECT_FALSE(actual_debug_marker_outer.has_begin_marker());

  ExpectDebugMarkerEndEq(actual_debug_marker_inner, kTimestamp2, kExpectedTextKeyInner,
                         expected_color, 1);
  ExpectDebugMarkerBeginEq(actual_debug_marker_inner, kTimestamp1, pre_submit_time,
                           post_submit_time, tid, pid);
}

TEST_F(SubmissionTrackerTest, CanRetrieveDebugMarkerAcrossTwoSubmissions) {
  ExpectSixNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_done_reading1;
  std::vector<uint32_t> actual_slots_done_reading2;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(2)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading1))
      .WillOnce(SaveArg<1>(&actual_slots_done_reading2));
  std::vector<uint32_t> actual_slots_marked_for_reset1;
  std::vector<uint32_t> actual_slots_marked_for_reset2;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsForReset)
      .Times(2)
      .WillOnce(SaveArg<1>(&actual_slots_marked_for_reset1))
      .WillOnce(SaveArg<1>(&actual_slots_marked_for_reset2));
  std::vector<orbit_grpc_protos::ProducerCaptureEvent> actual_capture_events;
  auto mock_enqueue_capture_event =
      [&actual_capture_events](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_events.emplace_back(std::move(capture_event));
        return true;
      };

  const char* text = "Text";
  constexpr uint64_t kExpectedTextKey = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text](const std::string& str) {
    EXPECT_STREQ(text, str.c_str());
    return kExpectedTextKey;
  };
  EXPECT_CALL(*producer_, InternStringIfNecessaryAndGetKey)
      .Times(1)
      .WillOnce(Invoke(mock_intern_string_if_necessary_and_get_key));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(2)
      .WillRepeatedly(Invoke(mock_enqueue_capture_event));

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text, expected_color);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  uint64_t pre_submit_time_1 = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional_1 =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_1);
  uint64_t post_submit_time_1 = orbit_base::CaptureTimestampNs();
  tracker_.CompleteSubmits(device_);
  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional_2 =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_2);
  tracker_.CompleteSubmits(device_);
  tracker_.ResetCommandBuffer(command_buffer_);

  EXPECT_THAT(actual_slots_done_reading1, UnorderedElementsAre(kSlotIndex1, kSlotIndex3));
  EXPECT_THAT(actual_slots_done_reading2,
              UnorderedElementsAre(kSlotIndex2, kSlotIndex4, kSlotIndex5, kSlotIndex6));
  EXPECT_THAT(actual_slots_marked_for_reset1,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3));
  EXPECT_THAT(actual_slots_marked_for_reset2,
              UnorderedElementsAre(kSlotIndex4, kSlotIndex5, kSlotIndex6));
  ASSERT_EQ(actual_capture_events.size(), 2);

  const orbit_grpc_protos::ProducerCaptureEvent& actual_capture_event_1 =
      actual_capture_events.at(0);
  EXPECT_TRUE(actual_capture_event_1.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_1 =
      actual_capture_event_1.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_1.num_begin_markers(), 1);
  EXPECT_EQ(actual_queue_submission_1.completed_markers_size(), 0);

  const orbit_grpc_protos::ProducerCaptureEvent& actual_capture_event_2 =
      actual_capture_events.at(1);
  EXPECT_TRUE(actual_capture_event_2.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_2 =
      actual_capture_event_2.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_2.num_begin_markers(), 0);
  EXPECT_EQ(actual_queue_submission_2.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker =
      actual_queue_submission_2.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker, kTimestamp5, kExpectedTextKey, expected_color, 0);
  ExpectDebugMarkerBeginEq(actual_debug_marker, kTimestamp2, pre_submit_time_1, post_submit_time_1,
                           tid, pid);
}

TEST_F(SubmissionTrackerTest, CanRetrieveDebugMarkerAcrossTwoSubmissionsEvenWhenBeginNotCaptured) {
  ExpectFourNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_done_reading1;
  std::vector<uint32_t> actual_slots_done_reading2;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(2)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading1))
      .WillOnce(SaveArg<1>(&actual_slots_done_reading2));

  std::vector<uint32_t> actual_slots_marked_for_reset1;
  std::vector<uint32_t> actual_slots_marked_for_reset2;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsForReset)
      .Times(2)
      .WillOnce(SaveArg<1>(&actual_slots_marked_for_reset1))
      .WillOnce(SaveArg<1>(&actual_slots_marked_for_reset2));
  std::vector<orbit_grpc_protos::ProducerCaptureEvent> actual_capture_events;
  auto mock_enqueue_capture_event =
      [&actual_capture_events](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_events.emplace_back(std::move(capture_event));
        return true;
      };

  const char* text = "Text";
  constexpr uint64_t kExpectedTextKey = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text](const std::string& str) {
    EXPECT_STREQ(text, str.c_str());
    return kExpectedTextKey;
  };
  EXPECT_CALL(*producer_, InternStringIfNecessaryAndGetKey)
      .Times(1)
      .WillOnce(Invoke(mock_intern_string_if_necessary_and_get_key));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(2)
      .WillRepeatedly(Invoke(mock_enqueue_capture_event));

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text, expected_color);
  producer_->StartCapture();
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional_1 =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_1);
  tracker_.CompleteSubmits(device_);
  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional_2 =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_2);
  tracker_.CompleteSubmits(device_);
  tracker_.ResetCommandBuffer(command_buffer_);

  EXPECT_THAT(actual_slots_done_reading1, UnorderedElementsAre(kSlotIndex1));
  EXPECT_THAT(actual_slots_done_reading2,
              UnorderedElementsAre(kSlotIndex2, kSlotIndex3, kSlotIndex4));
  EXPECT_THAT(actual_slots_marked_for_reset1, UnorderedElementsAre(kSlotIndex1));
  EXPECT_THAT(actual_slots_marked_for_reset2,
              UnorderedElementsAre(kSlotIndex2, kSlotIndex3, kSlotIndex4));
  ASSERT_EQ(actual_capture_events.size(), 2);

  const orbit_grpc_protos::ProducerCaptureEvent& actual_capture_event_1 =
      actual_capture_events.at(0);
  EXPECT_TRUE(actual_capture_event_1.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_1 =
      actual_capture_event_1.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_1.num_begin_markers(), 0);
  EXPECT_EQ(actual_queue_submission_1.completed_markers_size(), 0);

  const orbit_grpc_protos::ProducerCaptureEvent& actual_capture_event_2 =
      actual_capture_events.at(1);
  EXPECT_TRUE(actual_capture_event_2.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_2 =
      actual_capture_event_2.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_2.num_begin_markers(), 0);
  EXPECT_EQ(actual_queue_submission_2.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker =
      actual_queue_submission_2.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker, kTimestamp3, kExpectedTextKey, expected_color, 0);
  EXPECT_FALSE(actual_debug_marker.has_begin_marker());
}

TEST_F(SubmissionTrackerTest, ResetSlotsOnDebugMarkerAcrossTwoSubmissionsWhenEndNotCaptured) {
  ExpectThreeNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_done_reading1;
  std::vector<uint32_t> actual_slots_done_reading2;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(2)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading1))
      .WillOnce(SaveArg<1>(&actual_slots_done_reading2));

  std::vector<uint32_t> actual_slots_marked_for_reset1;
  std::vector<uint32_t> actual_slots_marked_for_reset2;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsForReset)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_marked_for_reset1));
  orbit_grpc_protos::ProducerCaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };

  const char* text = "Text";
  EXPECT_CALL(*producer_, InternStringIfNecessaryAndGetKey).Times(0);
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(1)
      .WillOnce(Invoke(mock_enqueue_capture_event));

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text, expected_color);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional_1 =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_1);
  tracker_.CompleteSubmits(device_);

  producer_->StopCapture();
  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional_2 =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_2);
  tracker_.CompleteSubmits(device_);
  tracker_.ResetCommandBuffer(command_buffer_);

  EXPECT_THAT(actual_slots_done_reading1, UnorderedElementsAre(kSlotIndex1, kSlotIndex3));
  EXPECT_THAT(actual_slots_done_reading2, UnorderedElementsAre(kSlotIndex2));

  EXPECT_THAT(actual_slots_marked_for_reset1,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3));

  EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
      actual_capture_event.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission.num_begin_markers(), 1);
  EXPECT_EQ(actual_queue_submission.completed_markers_size(), 0);
}

TEST_F(SubmissionTrackerTest, ResetDebugMarkerSlotsWhenStopBeforeASubmission) {
  ExpectFourNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults).Times(0);
  std::vector<uint32_t> actual_slots_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(0);
  const char* text = "Text";

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text, expected_color);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  producer_->StopCapture();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4));
}

TEST_F(SubmissionTrackerTest, CanLimitNestedDebugMarkerDepthPerCommandBuffer) {
  ExpectFourNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_slots_done_reading;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_done_reading));
  orbit_grpc_protos::ProducerCaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };

  const std::string text_outer = "Outer";
  const std::string text_inner = "Inner";
  constexpr uint64_t kExpectedTextKeyOuter = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text_outer](const std::string& str) {
    if (str == text_outer) {
      return kExpectedTextKeyOuter;
    }
    ORBIT_UNREACHABLE();
  };
  EXPECT_CALL(*producer_, InternStringIfNecessaryAndGetKey)
      .Times(1)
      .WillOnce(Invoke(mock_intern_string_if_necessary_and_get_key));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(1)
      .WillOnce(Invoke(mock_enqueue_capture_event));

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  constexpr uint64_t kMaxDepth = 1;
  producer_->StartCapture(kMaxDepth);
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_outer.c_str(), expected_color);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_inner.c_str(), expected_color);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint64_t pre_submit_time = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = orbit_base::CaptureTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_slots_done_reading,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4));
  EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
      actual_capture_event.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission.num_begin_markers(), 1);
  EXPECT_EQ(actual_queue_submission.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker_outer =
      actual_queue_submission.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker_outer, kTimestamp3, kExpectedTextKeyOuter,
                         expected_color, 0);
  ExpectDebugMarkerBeginEq(actual_debug_marker_outer, kTimestamp2, pre_submit_time,
                           post_submit_time, tid, pid);
}

TEST_F(SubmissionTrackerTest, CanLimitNestedDebugMarkerDepthPerCommandBufferAcrossSubmissions) {
  ExpectSevenNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));

  std::vector<uint32_t> actual_slots_done_reading;
  auto mock_mark_slots_done_reading = [&actual_slots_done_reading](
                                          VkDevice /*device*/,
                                          const std::vector<uint32_t>& slots_to_reset) {
    actual_slots_done_reading.insert(actual_slots_done_reading.end(), slots_to_reset.begin(),
                                     slots_to_reset.end());
  };
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsDoneReading)
      .WillRepeatedly(Invoke(mock_mark_slots_done_reading));

  std::vector<uint32_t> actual_slots_marked_for_reset;
  auto mock_mark_slots_for_reset = [&actual_slots_marked_for_reset](
                                       VkDevice /*device*/,
                                       const std::vector<uint32_t>& slots_to_reset) {
    actual_slots_marked_for_reset.insert(actual_slots_marked_for_reset.end(),
                                         slots_to_reset.begin(), slots_to_reset.end());
  };
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsForReset)
      .WillRepeatedly(Invoke(mock_mark_slots_for_reset));

  std::vector<orbit_grpc_protos::ProducerCaptureEvent> actual_capture_events;
  auto mock_enqueue_capture_event =
      [&actual_capture_events](orbit_grpc_protos::ProducerCaptureEvent&& capture_event) {
        actual_capture_events.emplace_back(std::move(capture_event));
        return true;
      };

  const std::string text_outer = "Outer";
  const std::string text_inner = "Inner";
  constexpr uint64_t kExpectedOuterTextKey = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text_outer](const std::string& str) {
    EXPECT_EQ(text_outer, str);
    return kExpectedOuterTextKey;
  };
  EXPECT_CALL(*producer_, InternStringIfNecessaryAndGetKey)
      .Times(1)
      .WillOnce(Invoke(mock_intern_string_if_necessary_and_get_key));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(2)
      .WillRepeatedly(Invoke(mock_enqueue_capture_event));

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  uint32_t tid = orbit_base::GetCurrentThreadId();
  uint32_t pid = orbit_base::GetCurrentProcessId();

  constexpr uint64_t kMaxDepth = 1;
  producer_->StartCapture(kMaxDepth);
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);  // timestamp 1
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_outer.c_str(),
                                expected_color);  // timestamp 2
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_inner.c_str(), expected_color);  // cut-off
  tracker_.MarkCommandBufferEnd(command_buffer_);  // timestamp 3
  uint64_t pre_submit_time_1 = orbit_base::CaptureTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional_1 =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_1);
  uint64_t post_submit_time_1 = orbit_base::CaptureTimestampNs();
  tracker_.CompleteSubmits(device_);

  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkCommandBufferBegin(command_buffer_);  // timestamp 4
  tracker_.MarkDebugMarkerEnd(command_buffer_);      // timestamp 5 - we can't know now to cut-off
  tracker_.MarkDebugMarkerEnd(command_buffer_);      // timestamp 6
  tracker_.MarkCommandBufferEnd(command_buffer_);    // timestamp 7
  std::optional<QueueSubmission> queue_submission_optional_2 =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_2);
  tracker_.CompleteSubmits(device_);
  tracker_.ResetCommandBuffer(command_buffer_);

  EXPECT_THAT(actual_slots_done_reading,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4, kSlotIndex5,
                                   kSlotIndex6, kSlotIndex7));
  EXPECT_THAT(actual_slots_marked_for_reset,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4, kSlotIndex5,
                                   kSlotIndex6, kSlotIndex7));
  ASSERT_EQ(actual_capture_events.size(), 2);

  const orbit_grpc_protos::ProducerCaptureEvent& actual_capture_event_1 =
      actual_capture_events.at(0);
  EXPECT_TRUE(actual_capture_event_1.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_1 =
      actual_capture_event_1.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_1.num_begin_markers(), 1);
  EXPECT_EQ(actual_queue_submission_1.completed_markers_size(), 0);

  const orbit_grpc_protos::ProducerCaptureEvent& actual_capture_event_2 =
      actual_capture_events.at(1);
  EXPECT_TRUE(actual_capture_event_2.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_2 =
      actual_capture_event_2.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_2.num_begin_markers(), 0);
  EXPECT_EQ(actual_queue_submission_2.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker =
      actual_queue_submission_2.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker, kTimestamp6, kExpectedOuterTextKey, expected_color,
                         0);
  ExpectDebugMarkerBeginEq(actual_debug_marker, kTimestamp2, pre_submit_time_1, post_submit_time_1,
                           tid, pid);
}

// ------------------------------------------------------------------------------------------------
// Tests to ensure that we can handle some common misuses of the command buffer lifecycle:
// ------------------------------------------------------------------------------------------------

TEST_F(SubmissionTrackerTest, MarkCommandBufferEndWontWriteTimestampsInInitialState) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(1)
      .WillOnce(Invoke(MockNextReadyQuerySlot1));
  std::vector<uint32_t> actual_slots_to_rollback;
  EXPECT_CALL(timer_query_pool_, RollbackPendingQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_to_rollback));
  EXPECT_CALL(dispatch_table_, CmdWriteTimestamp)
      .Times(1)
      .WillOnce(Return(dummy_write_timestamp_function));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);

  EXPECT_THAT(actual_slots_to_rollback, UnorderedElementsAre(kSlotIndex1));
}

TEST_F(SubmissionTrackerTest, MarkDebugMarkerBeginWontWriteTimestampsInInitialState) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(1)
      .WillOnce(Invoke(MockNextReadyQuerySlot1));
  std::vector<uint32_t> actual_slots_to_rollback;
  EXPECT_CALL(timer_query_pool_, RollbackPendingQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_to_rollback));
  EXPECT_CALL(dispatch_table_, CmdWriteTimestamp)
      .Times(1)
      .WillOnce(Return(dummy_write_timestamp_function));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, "Some Text", {});

  EXPECT_THAT(actual_slots_to_rollback, UnorderedElementsAre(kSlotIndex1));
}

TEST_F(SubmissionTrackerTest, MarkDebugMarkerEndWontWriteTimestampsInInitialState) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(2)
      .WillOnce(Invoke(MockNextReadyQuerySlot1))
      .WillOnce(Invoke(MockNextReadyQuerySlot2));
  std::vector<uint32_t> actual_slots_to_rollback;
  EXPECT_CALL(timer_query_pool_, RollbackPendingQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_to_rollback));
  EXPECT_CALL(dispatch_table_, CmdWriteTimestamp)
      .Times(2)
      .WillRepeatedly(Return(dummy_write_timestamp_function));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, "Some Text", {});
  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkDebugMarkerEnd(command_buffer_);

  EXPECT_THAT(actual_slots_to_rollback, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
}

TEST_F(SubmissionTrackerTest, SubmitCommandBufferInInitialStateWillNotYieldTimestamps) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(2)
      .WillOnce(Invoke(MockNextReadyQuerySlot1))
      .WillOnce(Invoke(MockNextReadyQuerySlot2));
  std::vector<uint32_t> actual_slots_to_rollback;
  EXPECT_CALL(timer_query_pool_, RollbackPendingQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_to_rollback));
  EXPECT_CALL(dispatch_table_, CmdWriteTimestamp)
      .Times(2)
      .WillRepeatedly(Return(dummy_write_timestamp_function));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  tracker_.ResetCommandBuffer(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);

  EXPECT_THAT(actual_slots_to_rollback, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));

  // Here we don't mind about the actual implementation. If the implementation detects that this
  // submission does not not contain any timestamp and thus drop the submission, this is fine.
  // Otherwise, we need to check, that the yielded submission does not contain a timestamp.
  if (!queue_submission_optional.has_value()) {
    return;
  }

  for (const auto& submit_info : queue_submission_optional->submit_infos) {
    EXPECT_TRUE(submit_info.command_buffers.empty());
  }
  EXPECT_TRUE(queue_submission_optional->completed_markers.empty());
}

TEST_F(SubmissionTrackerTest, WillWriteTimestampEvenWhenCommandBufferIsNotInInitialState) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(3)
      .WillOnce(Invoke(MockNextReadyQuerySlot1))
      .WillOnce(Invoke(MockNextReadyQuerySlot2))
      .WillOnce(Invoke(MockNextReadyQuerySlot3));
  std::vector<uint32_t> actual_slots_to_reset;
  EXPECT_CALL(timer_query_pool_, MarkQuerySlotsForReset)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_slots_to_reset));
  EXPECT_CALL(dispatch_table_, CmdWriteTimestamp)
      .Times(3)
      .WillRepeatedly(Return(dummy_write_timestamp_function));

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(queue_, 1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.MarkCommandBufferBegin(command_buffer_);

  EXPECT_THAT(actual_slots_to_reset, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
}
}  // namespace orbit_vulkan_layer
