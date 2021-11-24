// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef METRICS_UPLOADER_SCOPED_METRIC_H_
#define METRICS_UPLOADER_SCOPED_METRIC_H_

#include <chrono>

#include "MetricsUploader/MetricsUploader.h"
#include "MetricsUploader/orbit_log_event.pb.h"

namespace orbit_metrics_uploader {

class ScopedMetric {
 public:
  explicit ScopedMetric(MetricsUploader* uploader, OrbitLogEvent_LogEventType log_event_type);
  ScopedMetric(const ScopedMetric& other) = delete;
  ScopedMetric& operator=(const ScopedMetric& other) = delete;
  ScopedMetric(ScopedMetric&& other);
  ScopedMetric& operator=(ScopedMetric&& other);
  ~ScopedMetric();

  void SetStatusCode(OrbitLogEvent::StatusCode status_code) { status_code_ = status_code; }

 private:
  MetricsUploader* uploader_;
  OrbitLogEvent_LogEventType log_event_type_;
  OrbitLogEvent::StatusCode status_code_ = OrbitLogEvent::SUCCESS;
  std::chrono::steady_clock::time_point start_;
};

}  // namespace orbit_metrics_uploader

#endif  // METRICS_UPLOADER_SCOPED_METRIC_H_