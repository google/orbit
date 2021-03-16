// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "MetricsUploader/MetricsUploader.h"
#include "MetricsUploader/ScopedMetric.h"

namespace orbit_metrics_uploader {

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
};

TEST(ScopedMetric, Constructor) {
  { ScopedMetric metric{nullptr, OrbitLogEvent_LogEventType_ORBIT_MAIN_WINDOW_OPEN}; }

  MockUploader uploader{};

  EXPECT_CALL(uploader, SendLogEvent(OrbitLogEvent_LogEventType_ORBIT_MAIN_WINDOW_OPEN, _,
                                     OrbitLogEvent_StatusCode_SUCCESS))
      .Times(1);

  { ScopedMetric metric{&uploader, OrbitLogEvent_LogEventType_ORBIT_MAIN_WINDOW_OPEN}; }
}

TEST(ScopedMetric, SetStatusCode) {
  MockUploader uploader{};

  EXPECT_CALL(uploader, SendLogEvent(OrbitLogEvent_LogEventType_ORBIT_MAIN_WINDOW_OPEN, _,
                                     OrbitLogEvent_StatusCode_CANCELLED))
      .Times(1);

  {
    ScopedMetric metric{&uploader, OrbitLogEvent_LogEventType_ORBIT_MAIN_WINDOW_OPEN};
    metric.SetStatusCode(OrbitLogEvent_StatusCode_CANCELLED);
  }
}

TEST(ScopedMetric, Sleep) {
  MockUploader uploader{};

  std::chrono::milliseconds sleep_time{1};

  EXPECT_CALL(uploader, SendLogEvent(OrbitLogEvent_LogEventType_ORBIT_MAIN_WINDOW_OPEN,
                                     Ge(sleep_time), OrbitLogEvent_StatusCode_SUCCESS))
      .Times(1);

  {
    ScopedMetric metric{&uploader, OrbitLogEvent_LogEventType_ORBIT_MAIN_WINDOW_OPEN};
    std::this_thread::sleep_for(sleep_time);
  }
}

TEST(ScopedMetric, MoveAndSleep) {
  MockUploader uploader{};

  std::chrono::milliseconds sleep_time{1};

  EXPECT_CALL(uploader, SendLogEvent(OrbitLogEvent_LogEventType_ORBIT_MAIN_WINDOW_OPEN,
                                     Ge(sleep_time * 2), OrbitLogEvent_StatusCode_SUCCESS))
      .Times(1);

  {
    ScopedMetric metric{&uploader, OrbitLogEvent_LogEventType_ORBIT_MAIN_WINDOW_OPEN};
    std::this_thread::sleep_for(sleep_time);

    [metric = std::move(metric), sleep_time]() { std::this_thread::sleep_for(sleep_time); }();
  }
}

}  // namespace orbit_metrics_uploader