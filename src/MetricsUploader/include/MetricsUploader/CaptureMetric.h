// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef METRICS_UPLOADER_CAPTURE_METRIC_H_
#define METRICS_UPLOADER_CAPTURE_METRIC_H_

#include <chrono>

#include "MetricsUploader/MetricsUploader.h"
#include "orbit_log_event.pb.h"

namespace orbit_metrics_uploader {

struct CaptureStartData {
  int64_t number_of_instrumented_functions = 0;
  int64_t number_of_frame_tracks = 0;
  int64_t number_of_manual_start_timers = 0;
  int64_t number_of_manual_stop_timers = 0;
  int64_t number_of_manual_start_async_timers = 0;
  int64_t number_of_manual_stop_async_timers = 0;
  int64_t number_of_manual_tracked_values = 0;
  orbit_metrics_uploader::OrbitCaptureData_ThreadStates thread_states =
      orbit_metrics_uploader::OrbitCaptureData_ThreadStates_THREAD_STATES_UNKNOWN;
};

struct CaptureCompleteData {
  std::chrono::milliseconds duration_in_milliseconds = std::chrono::milliseconds{0};
};

class CaptureMetric {
 public:
  explicit CaptureMetric(MetricsUploader* uploader, const CaptureStartData& start_data);
  CaptureMetric(const CaptureMetric& other) = delete;
  CaptureMetric& operator=(const CaptureMetric& other) = delete;
  CaptureMetric(CaptureMetric&& other) noexcept = default;
  CaptureMetric& operator=(CaptureMetric&& other) noexcept = default;
  ~CaptureMetric() = default;

  void SetCaptureFailed();
  void SetCaptureCancelled();
  void SetCaptureComplete(const CaptureCompleteData& complete_data);
  bool Send();

 private:
  MetricsUploader* uploader_;
  OrbitCaptureData capture_data_;
  OrbitLogEvent_StatusCode status_code_ = OrbitLogEvent_StatusCode_UNKNOWN_STATUS;
  std::chrono::steady_clock::time_point start_;
};

}  // namespace orbit_metrics_uploader

#endif  // METRICS_UPLOADER_CAPTURE_METRIC_H_