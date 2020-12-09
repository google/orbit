// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SubmissionTracker.h"
#include "VulkanLayerProducer.h"
#include "absl/base/casts.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

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
  MOCK_METHOD(void, ResetQuerySlots, (VkDevice, const std::vector<uint32_t>&), ());
  MOCK_METHOD(void, RollbackPendingQuerySlots, (VkDevice, const std::vector<uint32_t>&), ());
  MOCK_METHOD(bool, NextReadyQuerySlot, (VkDevice, uint32_t*), ());
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
  MOCK_METHOD(bool, EnqueueCaptureEvent, (orbit_grpc_protos::CaptureEvent && capture_event),
              (override));

  MOCK_METHOD(void, BringUp, (const std::shared_ptr<grpc::Channel>& channel), (override));
  MOCK_METHOD(void, TakeDown, (), (override));

  MOCK_METHOD(void, SetCaptureStatusListener, (CaptureStatusListener*), (override));

  void StartCapture() {
    is_capturing_ = true;
    ASSERT_NE(listener_, nullptr);
    listener_->OnCaptureStart();
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
    tracker.SetVulkanLayerProducer(producer_.get());
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
                               .pCommandBuffers = &command_buffer_,
                               .commandBufferCount = 1};

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
        UNREACHABLE();
    }
    return VK_SUCCESS;
  };

  const PFN_vkGetQueryPoolResults mock_get_query_pool_results_function_not_ready_ =
      +[](VkDevice /*device*/, VkQueryPool /*queryPool*/, uint32_t /*first_query*/,
          uint32_t /*query_count*/, size_t /*dataSize*/, void* /*data*/, VkDeviceSize /*stride*/,
          VkQueryResultFlags /*flags*/) -> VkResult { return VK_NOT_READY; };

  static void ExpectSingleCommandBufferSubmissionEq(
      const orbit_grpc_protos::CaptureEvent& actual_capture_event, uint64_t test_pre_submit_time,
      uint64_t test_post_submit_time, pid_t expected_tid,
      uint64_t expected_command_buffer_begin_timestamp,
      uint64_t expected_command_buffer_end_timestamp) {
    EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
    const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
        actual_capture_event.gpu_queue_submission();

    ExpectSubmitEq(actual_queue_submission.meta_info(), test_pre_submit_time, test_post_submit_time,
                   expected_tid);

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
                             pid_t expected_tid) {
    EXPECT_LE(test_pre_submit_time, actual_meta_info.pre_submission_cpu_timestamp());
    EXPECT_LE(actual_meta_info.pre_submission_cpu_timestamp(),
              actual_meta_info.post_submission_cpu_timestamp());
    EXPECT_LE(actual_meta_info.post_submission_cpu_timestamp(), test_post_submit_time);
    EXPECT_EQ(expected_tid, actual_meta_info.tid());
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
                                       uint64_t test_post_submit_time, pid_t expected_tid) {
    ASSERT_TRUE(actual_debug_marker.has_begin_marker());
    EXPECT_EQ(actual_debug_marker.begin_marker().gpu_timestamp_ns(), expected_timestamp);
    const orbit_grpc_protos::GpuQueueSubmissionMetaInfo& actual_begin_marker_meta_info =
        actual_debug_marker.begin_marker().meta_info();
    ExpectSubmitEq(actual_begin_marker_meta_info, test_pre_submit_time, test_post_submit_time,
                   expected_tid);
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

TEST_F(SubmissionTrackerTest, MarkCommandBufferEndWillWriteTimestampsWhenNotCapturedBegin) {
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
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));
  orbit_grpc_protos::CaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::CaptureEvent&& capture_event) {
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
  pid_t pid = GetCurrentThreadId();
  uint64_t pre_submit_time = MonotonicTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = MonotonicTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
  ExpectSingleCommandBufferSubmissionEq(actual_capture_event, pre_submit_time, post_submit_time,
                                        pid, kTimestamp1, kTimestamp2);
}

TEST_F(SubmissionTrackerTest,
       CanRetrieveCommandBufferTimestampsForACompleteSubmissionAtSecondPresent) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillOnce(Return(mock_get_query_pool_results_function_not_ready_))
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));
  orbit_grpc_protos::CaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::CaptureEvent&& capture_event) {
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
  pid_t pid = GetCurrentThreadId();
  uint64_t pre_submit_time = MonotonicTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = MonotonicTimestampNs();
  tracker_.CompleteSubmits(device_);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
  ExpectSingleCommandBufferSubmissionEq(actual_capture_event, pre_submit_time, post_submit_time,
                                        pid, kTimestamp1, kTimestamp2);
}

TEST_F(SubmissionTrackerTest, StopCaptureBeforeSubmissionWillResetTheSlots) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults).Times(0);
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));

  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(0);

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  producer_->StopCapture();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
}

TEST_F(SubmissionTrackerTest,
       CommandBufferTimestampsRecordedWhenCapturingCanBeRetrievedWhenNotCapturing) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));
  orbit_grpc_protos::CaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::CaptureEvent&& capture_event) {
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
  pid_t pid = GetCurrentThreadId();
  uint64_t pre_submit_time = MonotonicTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = MonotonicTimestampNs();
  producer_->StopCapture();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
  ExpectSingleCommandBufferSubmissionEq(actual_capture_event, pre_submit_time, post_submit_time,
                                        pid, kTimestamp1, kTimestamp2);
}

TEST_F(SubmissionTrackerTest, StopCaptureDuringSubmissionWillStillYieldResults) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));
  orbit_grpc_protos::CaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::CaptureEvent&& capture_event) {
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
  pid_t pid = GetCurrentThreadId();
  uint64_t pre_submit_time = MonotonicTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  producer_->StopCapture();
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = MonotonicTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
  ExpectSingleCommandBufferSubmissionEq(actual_capture_event, pre_submit_time, post_submit_time,
                                        pid, kTimestamp1, kTimestamp2);
}

TEST_F(SubmissionTrackerTest, StartCaptureJustBeforeSubmissionWontWriteData) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot).Times(0);
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults).Times(0);
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots).Times(0);
  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(0);

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  producer_->StartCapture();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);
}

TEST_F(SubmissionTrackerTest, StartCaptureDuringSubmissionWontWriteData) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot).Times(0);
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults).Times(0);
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots).Times(0);
  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(0);

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  producer_->StartCapture();
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);
}

TEST_F(SubmissionTrackerTest, WillResetProperlyWhenStartStopAndStartACaptureWithinASubmission) {
  EXPECT_CALL(timer_query_pool_, NextReadyQuerySlot)
      .Times(1)
      .WillOnce(Invoke(MockNextReadyQuerySlot1));
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults).Times(0);
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(0);

  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  producer_->StartCapture();
  tracker_.MarkCommandBufferBegin(command_buffer_);
  producer_->StopCapture();
  tracker_.MarkCommandBufferEnd(command_buffer_);
  producer_->StartCapture();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots, UnorderedElementsAre(kSlotIndex1));
}

TEST_F(SubmissionTrackerTest, CannotReuseCommandBufferWithoutReset) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots).Times(1);
  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(1);

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);

  EXPECT_DEATH({ tracker_.MarkCommandBufferBegin(command_buffer_); }, "");
}

TEST_F(SubmissionTrackerTest, CanReuseCommandBufferAfterReset) {
  ExpectThreeNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots).Times(1);
  EXPECT_CALL(*producer_, EnqueueCaptureEvent).Times(1);

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
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
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));
  orbit_grpc_protos::CaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::CaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };

  const char* text = "Text";
  constexpr uint64_t expected_text_key = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text](std::string str) {
    EXPECT_STREQ(text, str.c_str());
    return expected_text_key;
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
  pid_t tid = GetCurrentThreadId();
  uint64_t pre_submit_time = MonotonicTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = MonotonicTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4));
  EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
      actual_capture_event.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission.num_begin_markers(), 1);
  EXPECT_EQ(actual_queue_submission.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker =
      actual_queue_submission.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker, kTimestamp3, expected_text_key, expected_color, 0);
  ExpectDebugMarkerBeginEq(actual_debug_marker, kTimestamp2, pre_submit_time, post_submit_time,
                           tid);
}

TEST_F(SubmissionTrackerTest, CanRetrieveDebugMarkerEndEvenWhenNotCapturedBegin) {
  ExpectTwoNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));
  orbit_grpc_protos::CaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::CaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };

  const char* text = "Text";
  constexpr uint64_t expected_text_key = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text](std::string str) {
    EXPECT_STREQ(text, str.c_str());
    return expected_text_key;
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
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots, UnorderedElementsAre(kSlotIndex1, kSlotIndex2));
  EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
      actual_capture_event.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission.num_begin_markers(), 0);
  EXPECT_EQ(actual_queue_submission.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker =
      actual_queue_submission.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker, kTimestamp1, expected_text_key, expected_color, 0);
  EXPECT_FALSE(actual_debug_marker.has_begin_marker());
}

TEST_F(SubmissionTrackerTest, CanRetrieveNextedDebugMarkerTimestampsForACompleteSubmission) {
  ExpectSixNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));
  orbit_grpc_protos::CaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::CaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };

  const std::string text_outer = "Outer";
  const std::string text_inner = "Inner";
  constexpr uint64_t expected_text_key_outer = 111;
  constexpr uint64_t expected_text_key_inner = 112;
  auto mock_intern_string_if_necessary_and_get_key = [&text_outer, &text_inner](std::string str) {
    if (str == text_outer) {
      return expected_text_key_outer;
    }
    if (str == text_inner) {
      return expected_text_key_inner;
    }
    UNREACHABLE();
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
  pid_t tid = GetCurrentThreadId();
  uint64_t pre_submit_time = MonotonicTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = MonotonicTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots, UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3,
                                                       kSlotIndex4, kSlotIndex5, kSlotIndex6));
  EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
      actual_capture_event.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission.num_begin_markers(), 2);
  EXPECT_EQ(actual_queue_submission.completed_markers_size(), 2);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker_inner =
      actual_queue_submission.completed_markers(0);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker_outer =
      actual_queue_submission.completed_markers(1);

  ExpectDebugMarkerEndEq(actual_debug_marker_outer, kTimestamp5, expected_text_key_outer,
                         expected_color, 0);
  ExpectDebugMarkerBeginEq(actual_debug_marker_outer, kTimestamp2, pre_submit_time,
                           post_submit_time, tid);

  ExpectDebugMarkerEndEq(actual_debug_marker_inner, kTimestamp4, expected_text_key_inner,
                         expected_color, 1);
  ExpectDebugMarkerBeginEq(actual_debug_marker_inner, kTimestamp3, pre_submit_time,
                           post_submit_time, tid);
}

TEST_F(SubmissionTrackerTest,
       CanRetrieveNextedDebugMarkerTimestampsForASubmissionMissingFirstBegin) {
  ExpectFourNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));
  orbit_grpc_protos::CaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::CaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };

  const std::string text_outer = "Outer";
  const std::string text_inner = "Inner";
  constexpr uint64_t expected_text_key_outer = 111;
  constexpr uint64_t expected_text_key_inner = 112;
  auto mock_intern_string_if_necessary_and_get_key = [&text_outer, &text_inner](std::string str) {
    if (str == text_outer) {
      return expected_text_key_outer;
    }
    if (str == text_inner) {
      return expected_text_key_inner;
    }
    UNREACHABLE();
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
  pid_t tid = GetCurrentThreadId();
  uint64_t pre_submit_time = MonotonicTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = MonotonicTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots,
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

  ExpectDebugMarkerEndEq(actual_debug_marker_outer, kTimestamp3, expected_text_key_outer,
                         expected_color, 0);
  EXPECT_FALSE(actual_debug_marker_outer.has_begin_marker());

  ExpectDebugMarkerEndEq(actual_debug_marker_inner, kTimestamp2, expected_text_key_inner,
                         expected_color, 1);
  ExpectDebugMarkerBeginEq(actual_debug_marker_inner, kTimestamp1, pre_submit_time,
                           post_submit_time, tid);
}

TEST_F(SubmissionTrackerTest, CanRetrieveDebugMarkerAcrossTwoSubmissions) {
  ExpectSixNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_reset_slots_1;
  std::vector<uint32_t> actual_reset_slots_2;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(2)
      .WillOnce(SaveArg<1>(&actual_reset_slots_1))
      .WillOnce(SaveArg<1>(&actual_reset_slots_2));
  std::vector<orbit_grpc_protos::CaptureEvent> actual_capture_events;
  auto mock_enqueue_capture_event =
      [&actual_capture_events](orbit_grpc_protos::CaptureEvent&& capture_event) {
        actual_capture_events.emplace_back(std::move(capture_event));
        return true;
      };

  const char* text = "Text";
  constexpr uint64_t expected_text_key = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text](std::string str) {
    EXPECT_STREQ(text, str.c_str());
    return expected_text_key;
  };
  EXPECT_CALL(*producer_, InternStringIfNecessaryAndGetKey)
      .Times(1)
      .WillOnce(Invoke(mock_intern_string_if_necessary_and_get_key));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(2)
      .WillRepeatedly(Invoke(mock_enqueue_capture_event));

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  pid_t tid = GetCurrentThreadId();

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text, expected_color);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  uint64_t pre_submit_time_1 = MonotonicTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional_1 =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_1);
  uint64_t post_submit_time_1 = MonotonicTimestampNs();
  tracker_.CompleteSubmits(device_);
  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional_2 =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_2);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots_1, UnorderedElementsAre(kSlotIndex1, kSlotIndex3));
  EXPECT_THAT(actual_reset_slots_2,
              UnorderedElementsAre(kSlotIndex2, kSlotIndex4, kSlotIndex5, kSlotIndex6));
  ASSERT_EQ(actual_capture_events.size(), 2);

  const orbit_grpc_protos::CaptureEvent& actual_capture_event_1 = actual_capture_events.at(0);
  EXPECT_TRUE(actual_capture_event_1.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_1 =
      actual_capture_event_1.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_1.num_begin_markers(), 1);
  EXPECT_EQ(actual_queue_submission_1.completed_markers_size(), 0);

  const orbit_grpc_protos::CaptureEvent& actual_capture_event_2 = actual_capture_events.at(1);
  EXPECT_TRUE(actual_capture_event_2.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_2 =
      actual_capture_event_2.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_2.num_begin_markers(), 0);
  EXPECT_EQ(actual_queue_submission_2.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker =
      actual_queue_submission_2.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker, kTimestamp5, expected_text_key, expected_color, 0);
  ExpectDebugMarkerBeginEq(actual_debug_marker, kTimestamp2, pre_submit_time_1, post_submit_time_1,
                           tid);
}

TEST_F(SubmissionTrackerTest, CanRetrieveDebugMarkerAcrossTwoSubmissionsEvenWhenNotCapturingBegin) {
  ExpectFourNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_reset_slots_1;
  std::vector<uint32_t> actual_reset_slots_2;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(2)
      .WillOnce(SaveArg<1>(&actual_reset_slots_1))
      .WillOnce(SaveArg<1>(&actual_reset_slots_2));
  std::vector<orbit_grpc_protos::CaptureEvent> actual_capture_events;
  auto mock_enqueue_capture_event =
      [&actual_capture_events](orbit_grpc_protos::CaptureEvent&& capture_event) {
        actual_capture_events.emplace_back(std::move(capture_event));
        return true;
      };

  const char* text = "Text";
  constexpr uint64_t expected_text_key = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text](std::string str) {
    EXPECT_STREQ(text, str.c_str());
    return expected_text_key;
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
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_1);
  tracker_.CompleteSubmits(device_);
  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional_2 =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_2);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots_1, UnorderedElementsAre(kSlotIndex1));
  EXPECT_THAT(actual_reset_slots_2, UnorderedElementsAre(kSlotIndex2, kSlotIndex3, kSlotIndex4));
  ASSERT_EQ(actual_capture_events.size(), 2);

  const orbit_grpc_protos::CaptureEvent& actual_capture_event_1 = actual_capture_events.at(0);
  EXPECT_TRUE(actual_capture_event_1.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_1 =
      actual_capture_event_1.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_1.num_begin_markers(), 0);
  EXPECT_EQ(actual_queue_submission_1.completed_markers_size(), 0);

  const orbit_grpc_protos::CaptureEvent& actual_capture_event_2 = actual_capture_events.at(1);
  EXPECT_TRUE(actual_capture_event_2.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_2 =
      actual_capture_event_2.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_2.num_begin_markers(), 0);
  EXPECT_EQ(actual_queue_submission_2.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker =
      actual_queue_submission_2.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker, kTimestamp3, expected_text_key, expected_color, 0);
  EXPECT_FALSE(actual_debug_marker.has_begin_marker());
}

TEST_F(SubmissionTrackerTest, ResetSlotsOnDebugMarkerAcrossTwoSubmissionsWhenNotCapturingEnd) {
  ExpectThreeNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_reset_slots_1;
  std::vector<uint32_t> actual_reset_slots_2;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(2)
      .WillOnce(SaveArg<1>(&actual_reset_slots_1))
      .WillOnce(SaveArg<1>(&actual_reset_slots_2));
  orbit_grpc_protos::CaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::CaptureEvent&& capture_event) {
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
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_1);
  tracker_.CompleteSubmits(device_);

  producer_->StopCapture();
  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkCommandBufferBegin(command_buffer_);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  std::optional<QueueSubmission> queue_submission_optional_2 =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_2);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots_1, UnorderedElementsAre(kSlotIndex1, kSlotIndex3));
  EXPECT_THAT(actual_reset_slots_2, UnorderedElementsAre(kSlotIndex2));

  EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
      actual_capture_event.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission.num_begin_markers(), 1);
  EXPECT_EQ(actual_queue_submission.completed_markers_size(), 0);
}

TEST_F(SubmissionTrackerTest, ResetDebugMarkerSlotsWhenStopBeforeASubmission) {
  ExpectFourNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults).Times(0);
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));
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
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4));
}

TEST_F(SubmissionTrackerTest, CanLimitNextedDebugMarkerDepthPerCommandBuffer) {
  tracker_.SetMaxLocalMarkerDepthPerCommandBuffer(1);

  ExpectFourNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));
  std::vector<uint32_t> actual_reset_slots;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots)
      .Times(1)
      .WillOnce(SaveArg<1>(&actual_reset_slots));
  orbit_grpc_protos::CaptureEvent actual_capture_event;
  auto mock_enqueue_capture_event =
      [&actual_capture_event](orbit_grpc_protos::CaptureEvent&& capture_event) {
        actual_capture_event = std::move(capture_event);
        return true;
      };

  const std::string text_outer = "Outer";
  const std::string text_inner = "Inner";
  constexpr uint64_t expected_text_key_outer = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text_outer](std::string str) {
    if (str == text_outer) {
      return expected_text_key_outer;
    }
    UNREACHABLE();
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
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_outer.c_str(), expected_color);
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_inner.c_str(), expected_color);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkDebugMarkerEnd(command_buffer_);
  tracker_.MarkCommandBufferEnd(command_buffer_);
  pid_t tid = GetCurrentThreadId();
  uint64_t pre_submit_time = MonotonicTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional);
  uint64_t post_submit_time = MonotonicTimestampNs();
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4));
  EXPECT_TRUE(actual_capture_event.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission =
      actual_capture_event.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission.num_begin_markers(), 1);
  EXPECT_EQ(actual_queue_submission.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker_outer =
      actual_queue_submission.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker_outer, kTimestamp3, expected_text_key_outer,
                         expected_color, 0);
  ExpectDebugMarkerBeginEq(actual_debug_marker_outer, kTimestamp2, pre_submit_time,
                           post_submit_time, tid);
}

TEST_F(SubmissionTrackerTest, CanLimitNextedDebugMarkerDepthPerCommandBufferAcrossSubmissions) {
  tracker_.SetMaxLocalMarkerDepthPerCommandBuffer(1);
  ExpectSevenNextReadyQuerySlotCalls();
  EXPECT_CALL(dispatch_table_, GetQueryPoolResults)
      .WillRepeatedly(Return(mock_get_query_pool_results_function_all_ready_));

  std::vector<uint32_t> actual_reset_slots;
  auto mock_reset_query_slots = [&actual_reset_slots](VkDevice /*device*/,
                                                      const std::vector<uint32_t> slots_to_reset) {
    actual_reset_slots.insert(actual_reset_slots.end(), slots_to_reset.begin(),
                              slots_to_reset.end());
  };
  std::vector<uint32_t> actual_reset_slots_2;
  EXPECT_CALL(timer_query_pool_, ResetQuerySlots).WillRepeatedly(Invoke(mock_reset_query_slots));

  std::vector<orbit_grpc_protos::CaptureEvent> actual_capture_events;
  auto mock_enqueue_capture_event =
      [&actual_capture_events](orbit_grpc_protos::CaptureEvent&& capture_event) {
        actual_capture_events.emplace_back(std::move(capture_event));
        return true;
      };

  const std::string text_outer = "Outer";
  const std::string text_inner = "Inner";
  constexpr uint64_t expected_outer_text_key = 111;
  auto mock_intern_string_if_necessary_and_get_key = [&text_outer](std::string str) {
    EXPECT_EQ(text_outer, str);
    return expected_outer_text_key;
  };
  EXPECT_CALL(*producer_, InternStringIfNecessaryAndGetKey)
      .Times(1)
      .WillOnce(Invoke(mock_intern_string_if_necessary_and_get_key));
  EXPECT_CALL(*producer_, EnqueueCaptureEvent)
      .Times(2)
      .WillRepeatedly(Invoke(mock_enqueue_capture_event));

  Color expected_color{1.f, 0.8f, 0.6f, 0.4f};

  pid_t tid = GetCurrentThreadId();

  producer_->StartCapture();
  tracker_.TrackCommandBuffers(device_, command_pool_, &command_buffer_, 1);
  tracker_.MarkCommandBufferBegin(command_buffer_);  // timestamp 1
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_outer.c_str(),
                                expected_color);  // timestamp 2
  tracker_.MarkDebugMarkerBegin(command_buffer_, text_inner.c_str(), expected_color);  // cut-off
  tracker_.MarkCommandBufferEnd(command_buffer_);  // timestamp 3
  uint64_t pre_submit_time_1 = MonotonicTimestampNs();
  std::optional<QueueSubmission> queue_submission_optional_1 =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_1);
  uint64_t post_submit_time_1 = MonotonicTimestampNs();
  tracker_.CompleteSubmits(device_);

  tracker_.ResetCommandBuffer(command_buffer_);
  tracker_.MarkCommandBufferBegin(command_buffer_);  // timestamp 4
  tracker_.MarkDebugMarkerEnd(command_buffer_);      // timestamp 5 - we can't know now to cut-off
  tracker_.MarkDebugMarkerEnd(command_buffer_);      // timestamp 6
  tracker_.MarkCommandBufferEnd(command_buffer_);    // timestamp 7
  std::optional<QueueSubmission> queue_submission_optional_2 =
      tracker_.PersistCommandBuffersOnSubmit(1, &submit_info_);
  tracker_.PersistDebugMarkersOnSubmit(queue_, 1, &submit_info_, queue_submission_optional_2);
  tracker_.CompleteSubmits(device_);

  EXPECT_THAT(actual_reset_slots,
              UnorderedElementsAre(kSlotIndex1, kSlotIndex2, kSlotIndex3, kSlotIndex4, kSlotIndex5,
                                   kSlotIndex6, kSlotIndex7));
  ASSERT_EQ(actual_capture_events.size(), 2);

  const orbit_grpc_protos::CaptureEvent& actual_capture_event_1 = actual_capture_events.at(0);
  EXPECT_TRUE(actual_capture_event_1.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_1 =
      actual_capture_event_1.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_1.num_begin_markers(), 1);
  EXPECT_EQ(actual_queue_submission_1.completed_markers_size(), 0);

  const orbit_grpc_protos::CaptureEvent& actual_capture_event_2 = actual_capture_events.at(1);
  EXPECT_TRUE(actual_capture_event_2.has_gpu_queue_submission());
  const orbit_grpc_protos::GpuQueueSubmission& actual_queue_submission_2 =
      actual_capture_event_2.gpu_queue_submission();
  EXPECT_EQ(actual_queue_submission_2.num_begin_markers(), 0);
  EXPECT_EQ(actual_queue_submission_2.completed_markers_size(), 1);
  const orbit_grpc_protos::GpuDebugMarker& actual_debug_marker =
      actual_queue_submission_2.completed_markers(0);

  ExpectDebugMarkerEndEq(actual_debug_marker, kTimestamp6, expected_outer_text_key, expected_color,
                         0);
  ExpectDebugMarkerBeginEq(actual_debug_marker, kTimestamp2, pre_submit_time_1, post_submit_time_1,
                           tid);
}

}  // namespace orbit_vulkan_layer