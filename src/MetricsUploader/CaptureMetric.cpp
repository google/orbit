// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MetricsUploader/CaptureMetric.h"

#include <chrono>
#include <cstdint>
#include <filesystem>

#include "MetricsUploader/orbit_log_event.pb.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"

namespace orbit_metrics_uploader {

CaptureMetric::CaptureMetric(MetricsUploader* uploader, const CaptureStartData& start_data)
    : uploader_(uploader), start_(std::chrono::steady_clock::now()) {
  CHECK(uploader_ != nullptr);
  capture_data_.set_number_of_instrumented_functions(start_data.number_of_instrumented_functions);
  capture_data_.set_number_of_frame_tracks(start_data.number_of_frame_tracks);
  capture_data_.set_thread_states(start_data.thread_states);
  capture_data_.set_memory_information_sampling_period_ms(
      start_data.memory_information_sampling_period_ms);
  capture_data_.set_lib_orbit_vulkan_layer(start_data.lib_orbit_vulkan_layer);
  capture_data_.set_local_marker_depth_per_command_buffer(
      start_data.local_marker_depth_per_command_buffer);
  capture_data_.set_max_local_marker_depth_per_command_buffer(
      start_data.max_local_marker_depth_per_command_buffer);
  capture_data_.set_dynamic_instrumentation_method(start_data.dynamic_instrumentation_method);
  capture_data_.set_callstack_samples_per_second(start_data.callstack_samples_per_second);
  capture_data_.set_callstack_unwinding_method(start_data.callstack_unwinding_method);
}

void CaptureMetric::SetCaptureCompleteData(const CaptureCompleteData& complete_data) {
  capture_data_.set_number_of_instrumented_function_timers(
      complete_data.number_of_instrumented_function_timers);
  capture_data_.set_number_of_gpu_activity_timers(complete_data.number_of_gpu_activity_timers);
  capture_data_.set_number_of_vulkan_layer_gpu_command_buffer_timers(
      complete_data.number_of_vulkan_layer_gpu_command_buffer_timers);
  capture_data_.set_number_of_vulkan_layer_gpu_debug_marker_timers(
      complete_data.number_of_vulkan_layer_gpu_debug_marker_timers);
  capture_data_.set_number_of_manual_start_timers(complete_data.number_of_manual_start_timers);
  capture_data_.set_number_of_manual_stop_timers(complete_data.number_of_manual_stop_timers);
  capture_data_.set_number_of_manual_start_async_timers(
      complete_data.number_of_manual_start_async_timers);
  capture_data_.set_number_of_manual_stop_async_timers(
      complete_data.number_of_manual_stop_async_timers);
  capture_data_.set_number_of_manual_tracked_value_timers(
      complete_data.number_of_manual_tracked_value_timers);
  file_path_ = complete_data.file_path;
}

bool CaptureMetric::SendCaptureFailed() {
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start_);
  capture_data_.set_duration_in_milliseconds(duration.count());
  status_code_ = OrbitLogEvent::INTERNAL_ERROR;
  return uploader_->SendCaptureEvent(capture_data_, status_code_);
}

bool CaptureMetric::SendCaptureCancelled() {
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start_);
  capture_data_.set_duration_in_milliseconds(duration.count());
  status_code_ = OrbitLogEvent::CANCELLED;
  return uploader_->SendCaptureEvent(capture_data_, status_code_);
}

bool CaptureMetric::SendCaptureSucceeded(std::chrono::milliseconds duration_in_milliseconds) {
  capture_data_.set_duration_in_milliseconds(duration_in_milliseconds.count());
  status_code_ = OrbitLogEvent::SUCCESS;

  if (file_path_.empty()) {
    ERROR("Unable to determine capture file size for metrics. File path is empty");
  } else {
    ErrorMessageOr<uint64_t> file_size = orbit_base::FileSize(file_path_);
    if (file_size.has_error()) {
      ERROR("Unable to determine capture file size for metrics. File: \"%s\"; error: %s",
            file_path_.string(), file_size.error().message());
    } else {
      capture_data_.set_file_size(file_size.value());
    }
  }

  return uploader_->SendCaptureEvent(capture_data_, status_code_);
}

}  // namespace orbit_metrics_uploader