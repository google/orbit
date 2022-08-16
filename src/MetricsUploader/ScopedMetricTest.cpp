// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "MetricsUploader/MetricsUploader.h"
#include "MetricsUploader/MockMetricsUploader.h"
#include "MetricsUploader/ScopedMetric.h"

namespace orbit_metrics_uploader {

using ::testing::_;
using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Lt;

TEST(ScopedMetric, Constructor) {
  { ScopedMetric metric{nullptr, OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN}; }

  MockMetricsUploader uploader{};

  EXPECT_CALL(uploader,
              SendLogEvent(OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN, _, OrbitLogEvent::SUCCESS))
      .Times(1);

  { ScopedMetric metric{&uploader, OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN}; }
}

TEST(ScopedMetric, SetStatusCode) {
  MockMetricsUploader uploader{};

  EXPECT_CALL(uploader,
              SendLogEvent(OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN, _, OrbitLogEvent::CANCELLED))
      .Times(1);

  {
    ScopedMetric metric{&uploader, OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN};
    metric.SetStatusCode(OrbitLogEvent::CANCELLED);
  }
}

TEST(ScopedMetric, Sleep) {
  MockMetricsUploader uploader{};

  std::chrono::milliseconds sleep_time{1};

  EXPECT_CALL(uploader, SendLogEvent(OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN, Ge(sleep_time),
                                     OrbitLogEvent::SUCCESS))
      .Times(1);

  {
    ScopedMetric metric{&uploader, OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN};
    std::this_thread::sleep_for(sleep_time);
  }
}

TEST(ScopedMetric, MoveAndSleep) {
  MockMetricsUploader uploader{};

  std::chrono::milliseconds sleep_time{1};

  EXPECT_CALL(uploader, SendLogEvent(OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN, Ge(sleep_time * 2),
                                     OrbitLogEvent::SUCCESS))
      .Times(1);

  {
    ScopedMetric metric{&uploader, OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN};
    std::this_thread::sleep_for(sleep_time);

    [metric = std::move(metric), sleep_time]() { std::this_thread::sleep_for(sleep_time); }();
  }
}

TEST(ScopedMetric, PauseAndResume) {
  MockMetricsUploader uploader{};

  std::chrono::milliseconds sleep_time{200};

  EXPECT_CALL(uploader,
              SendLogEvent(OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN,
                           AllOf(Ge(sleep_time), Lt(sleep_time * 2)), OrbitLogEvent::SUCCESS))
      .Times(3);

  {
    ScopedMetric metric{&uploader, OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN};
    std::this_thread::sleep_for(sleep_time / 2);

    metric.Pause();
    std::this_thread::sleep_for(sleep_time);
    metric.Resume();
    std::this_thread::sleep_for(sleep_time / 2);
  }

  {
    ScopedMetric metric{&uploader, OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN};
    std::this_thread::sleep_for(sleep_time);

    metric.Pause();
    std::this_thread::sleep_for(sleep_time);
  }

  {
    ScopedMetric metric{&uploader, OrbitLogEvent::ORBIT_MAIN_WINDOW_OPEN};
    std::this_thread::sleep_for(sleep_time / 2);

    metric.Pause();
    ScopedMetric moved_metric{std::move(metric)};
    std::this_thread::sleep_for(sleep_time);
    moved_metric.Resume();
    std::this_thread::sleep_for(sleep_time / 2);
  }
}

}  // namespace orbit_metrics_uploader