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
    3 /*number_of_manual_start_timers*/,
    4 /*number_of_manual_stop_timers*/,
    5 /*number_of_manual_start_async_timers*/,
    6 /*number_of_manual_stop_async_timers*/,
    7 /*number_of_manual_tracked_values*/,
    OrbitCaptureData_ThreadStates_THREAD_STATES_ENABLED /*thread_states*/,
    10 /*memory_information_sampling_period_ms*/
};

bool HasSameCaptureStartData(const OrbitCaptureData& capture_data,
                             const CaptureStartData& start_data) {
  return capture_data.number_of_instrumented_functions() ==
             start_data.number_of_instrumented_functions &&
         capture_data.number_of_frame_tracks() == start_data.number_of_frame_tracks &&
         capture_data.number_of_manual_start_timers() == start_data.number_of_manual_start_timers &&
         capture_data.number_of_manual_stop_timers() == start_data.number_of_manual_stop_timers &&
         capture_data.number_of_manual_start_async_timers() ==
             start_data.number_of_manual_start_async_timers &&
         capture_data.number_of_manual_stop_async_timers() ==
             start_data.number_of_manual_stop_async_timers &&
         capture_data.number_of_manual_tracked_values() ==
             start_data.number_of_manual_tracked_values &&
         capture_data.thread_states() == start_data.thread_states &&
         capture_data.memory_information_sampling_period_ms() ==
             start_data.memory_information_sampling_period_ms;
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

TEST(CaptureMetric, SendEmpty) {
  MockUploader uploader{};
  CaptureMetric metric{&uploader, kTestStartData};
  EXPECT_FALSE(metric.Send());
}

TEST(CaptureMetric, SetCaptureFailedAndSend) {
  MockUploader uploader{};

  EXPECT_CALL(uploader, SendCaptureEvent(_, _))
      .Times(1)
      .WillOnce(
          [](const OrbitCaptureData& capture_data, OrbitLogEvent_StatusCode status_code) -> bool {
            EXPECT_EQ(status_code, OrbitLogEvent_StatusCode_INTERNAL_ERROR);
            EXPECT_GE(capture_data.duration_in_milliseconds(), 5);
            EXPECT_TRUE(HasSameCaptureStartData(capture_data, kTestStartData));
            return true;
          });

  CaptureMetric metric{&uploader, kTestStartData};
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  metric.SetCaptureFailed();
  EXPECT_TRUE(metric.Send());
}

TEST(CaptureMetric, SetCaptureCancelledAndSend) {
  MockUploader uploader{};

  EXPECT_CALL(uploader, SendCaptureEvent(_, _))
      .Times(1)
      .WillOnce(
          [](const OrbitCaptureData& capture_data, OrbitLogEvent_StatusCode status_code) -> bool {
            EXPECT_EQ(status_code, OrbitLogEvent_StatusCode_CANCELLED);
            EXPECT_GE(capture_data.duration_in_milliseconds(), 5);
            EXPECT_TRUE(HasSameCaptureStartData(capture_data, kTestStartData));
            return true;
          });

  CaptureMetric metric{&uploader, kTestStartData};
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  metric.SetCaptureCancelled();
  EXPECT_TRUE(metric.Send());
}

TEST(CaptureMetric, SetCaptureCompleteAndSend) {
  MockUploader uploader{};

  CaptureCompleteData complete_data{
      std::chrono::milliseconds{51} /*duration_in_milliseconds*/
  };

  EXPECT_CALL(uploader, SendCaptureEvent(_, _))
      .Times(1)
      .WillOnce([complete_data](const OrbitCaptureData& capture_data,
                                OrbitLogEvent_StatusCode status_code) -> bool {
        EXPECT_EQ(status_code, OrbitLogEvent_StatusCode_SUCCESS);
        EXPECT_EQ(capture_data.duration_in_milliseconds(),
                  complete_data.duration_in_milliseconds.count());
        EXPECT_TRUE(HasSameCaptureStartData(capture_data, kTestStartData));
        return true;
      });

  CaptureMetric metric{&uploader, kTestStartData};
  metric.SetCaptureComplete(complete_data);
  EXPECT_TRUE(metric.Send());
}

TEST(CaptureMetric, MultipleSetAndSend) {
  MockUploader uploader{};

  CaptureCompleteData complete_data1{
      std::chrono::milliseconds{51} /*duration_in_milliseconds*/
  };

  CaptureCompleteData complete_data2{
      std::chrono::milliseconds{423} /*duration_in_milliseconds*/
  };

  EXPECT_CALL(uploader, SendCaptureEvent(_, _))
      .Times(1)
      .WillOnce([complete_data2](const OrbitCaptureData& capture_data,
                                 OrbitLogEvent_StatusCode status_code) -> bool {
        EXPECT_EQ(status_code, OrbitLogEvent_StatusCode_SUCCESS);
        EXPECT_EQ(capture_data.duration_in_milliseconds(),
                  complete_data2.duration_in_milliseconds.count());
        EXPECT_TRUE(HasSameCaptureStartData(capture_data, kTestStartData));
        return true;
      });

  CaptureMetric metric{&uploader, kTestStartData};
  metric.SetCaptureComplete(complete_data1);
  metric.SetCaptureFailed();
  metric.SetCaptureCancelled();
  metric.SetCaptureComplete(complete_data2);
  EXPECT_TRUE(metric.Send());
}

}  // namespace orbit_metrics_uploader