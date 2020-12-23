// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "MetricsUploader/MetricsUploader.h"

using orbit_metrics_uploader::MetricsUploader;

TEST(MetricsUploader, CreateMetricsUploaderFromClientWithoutSendEvent) {
  auto metrics_uploader =
      MetricsUploader::CreateMetricsUploader("MetricsUploaderClientWithoutSendEvent");
  EXPECT_EQ(metrics_uploader.has_value(), false);
}

TEST(MetricsUploader, CreateMetricsUploaderFromClientWithoutStart) {
  auto metrics_uploader =
      MetricsUploader::CreateMetricsUploader("MetricsUploaderClientWithoutStart");
  EXPECT_EQ(metrics_uploader.has_value(), false);
}

TEST(MetricsUploader, StartMetricsUploaderWithError) {
  auto metrics_uploader =
      MetricsUploader::CreateMetricsUploader("MetricsUploaderStartWithErrorClient");
  EXPECT_EQ(metrics_uploader.has_value(), false);
}

TEST(MetricsUploader, SendLogEvent) {
  auto metrics_uploader = MetricsUploader::CreateMetricsUploader("MetricsUploaderCompleteClient");
  EXPECT_EQ(metrics_uploader.has_value(), true);
  bool result = metrics_uploader.value().SendLogEvent(
      orbit_metrics_uploader::OrbitLogEvent_LogEventType_UNKNOWN_EVENT_TYPE);
  EXPECT_EQ(result, false);
  result = metrics_uploader.value().SendLogEvent(
      orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_INITIALIZED);
  EXPECT_EQ(result, true);
  result = metrics_uploader.value().SendLogEvent(
      orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_CAPTURE_DURATION,
      std::chrono::milliseconds(100));
  EXPECT_EQ(result, true);
}

TEST(MetricsUploader, CreateTwoMetricsUploaders) {
  auto metrics_uploader1 = MetricsUploader::CreateMetricsUploader("MetricsUploaderCompleteClient");
  EXPECT_EQ(metrics_uploader1.has_value(), true);
  auto metrics_uploader2 = MetricsUploader::CreateMetricsUploader("MetricsUploaderCompleteClient");
  EXPECT_EQ(metrics_uploader2.has_value(), false);
}
