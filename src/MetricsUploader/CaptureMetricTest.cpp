// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "MetricsUploader/CaptureMetric.h"
#include "MetricsUploader/MetricsUploader.h"
#include "orbit_log_event.pb.h"

namespace orbit_metrics_uploader {

namespace {

constexpr CaptureStartData kTestStartData{
    1 /*number_of_instrumented_functions*/,
    2 /*number_of_frame_tracks*/,
    3 /*number_of_manual_start_functions*/,
    4 /*number_of_manual_stop_functions*/,
    5 /*number_of_manual_start_async_functions*/,
    6 /*number_of_manual_stop_async_functions*/,
    7 /*number_of_manual_tracked_values*/,
    OrbitCaptureData_ThreadStates_THREAD_STATES_ENABLED /*thread_states*/,
    10 /*memory_information_sampling_period_ms*/,
    OrbitCaptureData_LibOrbitVulkanLayer_LIB_LOADED /*lib_orbit_vulkan_layer*/,
    OrbitCaptureData_LocalMarkerDepthPerCommandBuffer_LIMITED /*local_marker_depth_per_command_buffer*/
    ,
    11 /*max_local_marker_depth_per_command_buffer*/
};

constexpr CaptureCompleteData kTestCompleteData{
    101 /*number_of_instrumented_function_timers*/, 102 /*number_of_gpu_activity_timers*/,
    103 /*number_of_vulkan_layer_gpu_command_buffer_timers*/,
    104 /*number_of_vulkan_layer_gpu_debug_marker_timers*/
};

bool HasSameCaptureStartData(const OrbitCaptureData& capture_data,
                             const CaptureStartData& start_data) {
  return capture_data.number_of_instrumented_functions() ==
             start_data.number_of_instrumented_functions &&
         capture_data.number_of_frame_tracks() == start_data.number_of_frame_tracks &&
         capture_data.number_of_manual_start_functions() ==
             start_data.number_of_manual_start_functions &&
         capture_data.number_of_manual_stop_functions() ==
             start_data.number_of_manual_stop_functions &&
         capture_data.number_of_manual_start_async_functions() ==
             start_data.number_of_manual_start_async_functions &&
         capture_data.number_of_manual_stop_async_functions() ==
             start_data.number_of_manual_stop_async_functions &&
         capture_data.number_of_manual_tracked_values() ==
             start_data.number_of_manual_tracked_values &&
         capture_data.thread_states() == start_data.thread_states &&
         capture_data.memory_information_sampling_period_ms() ==
             start_data.memory_information_sampling_period_ms;
}

bool HasSameCaptureCompleteData(const OrbitCaptureData& capture_data,
                                const CaptureCompleteData& complete_data) {
  return capture_data.number_of_instrumented_function_timers() ==
             complete_data.number_of_instrumented_function_timers &&
         capture_data.number_of_gpu_activity_timers() ==
             complete_data.number_of_gpu_activity_timers &&
         capture_data.number_of_vulkan_layer_gpu_command_buffer_timers() ==
             complete_data.number_of_vulkan_layer_gpu_command_buffer_timers &&
         capture_data.number_of_vulkan_layer_gpu_debug_marker_timers() ==
             complete_data.number_of_vulkan_layer_gpu_debug_marker_timers;
}

}  // namespace

using ::testing::_;
using ::testing::Ge;

class MockUploader : public MetricsUploader {
 public:
  MOCK_METHOD(bool, SendLogEvent, (OrbitLogEvent_LogEventType /*log_event_type*/), (override));
  MOCK_METHOD(bool, SendLogEvent,
              (OrbitLogEvent_LogEventType /*log_event_type*/,
               std::chrono::milliseconds /*event_duration*/),
              (override));
  MOCK_METHOD(bool, SendLogEvent,
              (OrbitLogEvent_LogEventType /*log_event_type*/,
               std::chrono::milliseconds /*event_duration*/,
               OrbitLogEvent_StatusCode /*status_code*/),
              (override));
  MOCK_METHOD(bool, SendCaptureEvent,
              (OrbitCaptureData /*capture data*/, OrbitLogEvent_StatusCode /*status_code*/),
              (override));
};

TEST(CaptureMetric, SendCaptureFailed) {
  MockUploader uploader{};

  EXPECT_CALL(uploader, SendCaptureEvent(_, _))
      .Times(1)
      .WillOnce(
          [](const OrbitCaptureData& capture_data, OrbitLogEvent_StatusCode status_code) -> bool {
            EXPECT_EQ(status_code, OrbitLogEvent_StatusCode_INTERNAL_ERROR);
            EXPECT_GE(capture_data.duration_in_milliseconds(), 5);
            EXPECT_TRUE(HasSameCaptureStartData(capture_data, kTestStartData));
            EXPECT_TRUE(HasSameCaptureCompleteData(capture_data, kTestCompleteData));
            return true;
          });

  CaptureMetric metric{&uploader, kTestStartData};
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  metric.SetCaptureCompleteData(kTestCompleteData);
  EXPECT_TRUE(metric.SendCaptureFailed());
}

TEST(CaptureMetric, SendCaptureCancelled) {
  MockUploader uploader{};

  EXPECT_CALL(uploader, SendCaptureEvent(_, _))
      .Times(1)
      .WillOnce(
          [](const OrbitCaptureData& capture_data, OrbitLogEvent_StatusCode status_code) -> bool {
            EXPECT_EQ(status_code, OrbitLogEvent_StatusCode_CANCELLED);
            EXPECT_GE(capture_data.duration_in_milliseconds(), 5);
            EXPECT_TRUE(HasSameCaptureStartData(capture_data, kTestStartData));
            EXPECT_TRUE(HasSameCaptureCompleteData(capture_data, kTestCompleteData));
            return true;
          });

  CaptureMetric metric{&uploader, kTestStartData};
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  metric.SetCaptureCompleteData(kTestCompleteData);
  EXPECT_TRUE(metric.SendCaptureCancelled());
}

TEST(CaptureMetric, SendCaptureSucceeded) {
  MockUploader uploader{};

  EXPECT_CALL(uploader, SendCaptureEvent(_, _))
      .Times(1)
      .WillOnce(
          [](const OrbitCaptureData& capture_data, OrbitLogEvent_StatusCode status_code) -> bool {
            EXPECT_EQ(status_code, OrbitLogEvent_StatusCode_SUCCESS);
            EXPECT_EQ(capture_data.duration_in_milliseconds(), 51);
            EXPECT_TRUE(HasSameCaptureStartData(capture_data, kTestStartData));
            EXPECT_TRUE(HasSameCaptureCompleteData(capture_data, kTestCompleteData));
            return true;
          });

  CaptureMetric metric{&uploader, kTestStartData};
  metric.SetCaptureCompleteData(kTestCompleteData);
  EXPECT_TRUE(metric.SendCaptureSucceeded(std::chrono::milliseconds{51}));
}

TEST(CaptureMetric, SendCaptureSucceededWithoutCompleteData) {
  MockUploader uploader{};

  EXPECT_CALL(uploader, SendCaptureEvent(_, _))
      .Times(1)
      .WillOnce(
          [](const OrbitCaptureData& capture_data, OrbitLogEvent_StatusCode status_code) -> bool {
            EXPECT_EQ(status_code, OrbitLogEvent_StatusCode_SUCCESS);
            EXPECT_EQ(capture_data.duration_in_milliseconds(), 5);
            EXPECT_TRUE(HasSameCaptureStartData(capture_data, kTestStartData));
            EXPECT_TRUE(HasSameCaptureCompleteData(capture_data, {}));
            return true;
          });

  CaptureMetric metric{&uploader, kTestStartData};

  EXPECT_TRUE(metric.SendCaptureSucceeded(std::chrono::milliseconds{5}));
}

}  // namespace orbit_metrics_uploader