// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MetricsUploader/CaptureMetric.h"

#include <chrono>

#include "OrbitBase/Logging.h"
#include "orbit_log_event.pb.h"

namespace orbit_metrics_uploader {

CaptureMetric::CaptureMetric(MetricsUploader* uploader, const CaptureStartData& start_data)
    : uploader_(uploader), start_(std::chrono::steady_clock::now()) {
  CHECK(uploader_ != nullptr);
  capture_data_.set_number_of_instrumented_functions(start_data.number_of_instrumented_functions);
  capture_data_.set_number_of_frame_tracks(start_data.number_of_frame_tracks);
  capture_data_.set_number_of_manual_start_timers(start_data.number_of_manual_start_timers);
  capture_data_.set_number_of_manual_stop_timers(start_data.number_of_manual_stop_timers);
  capture_data_.set_number_of_manual_start_async_timers(
      start_data.number_of_manual_start_async_timers);
  capture_data_.set_number_of_manual_stop_async_timers(
      start_data.number_of_manual_stop_async_timers);
  capture_data_.set_number_of_manual_tracked_values(start_data.number_of_manual_tracked_values);
  capture_data_.set_thread_states(start_data.thread_states);
  capture_data_.set_memory_information_sampling_period_ms(
      start_data.memory_information_sampling_period_ms);
  capture_data_.set_lib_orbit_vulkan_layer(start_data.lib_orbit_vulkan_layer);
}

void CaptureMetric::SetCaptureFailed() {
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start_);
  capture_data_.set_duration_in_milliseconds(duration.count());
  status_code_ = OrbitLogEvent_StatusCode_INTERNAL_ERROR;
}

void CaptureMetric::SetCaptureCancelled() {
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start_);
  capture_data_.set_duration_in_milliseconds(duration.count());
  status_code_ = OrbitLogEvent_StatusCode_CANCELLED;
}

void CaptureMetric::SetCaptureComplete(const CaptureCompleteData& complete_data) {
  capture_data_.set_duration_in_milliseconds(complete_data.duration_in_milliseconds.count());
  status_code_ = OrbitLogEvent_StatusCode_SUCCESS;
}

bool CaptureMetric::Send() {
  if (status_code_ == OrbitLogEvent_StatusCode_UNKNOWN_STATUS) return false;
  return uploader_->SendCaptureEvent(capture_data_, status_code_);
}

}  // namespace orbit_metrics_uploader