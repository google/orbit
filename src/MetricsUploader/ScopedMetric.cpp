// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MetricsUploader/ScopedMetric.h"

#include <chrono>

namespace orbit_metrics_uploader {

ScopedMetric::ScopedMetric(MetricsUploader* uploader, OrbitLogEvent::LogEventType log_event_type)
    : uploader_(uploader),
      log_event_type_(log_event_type),
      start_(std::chrono::steady_clock::now()) {}

ScopedMetric::~ScopedMetric() {
  if (uploader_ == nullptr) return;

  // Update the pause_duration_ if the ScopedMetric is still being paused.
  Resume();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - start_) -
                  pause_duration_;
  uploader_->SendLogEvent(log_event_type_, duration, status_code_);
}

ScopedMetric::ScopedMetric(ScopedMetric&& other)
    : uploader_(other.uploader_),
      log_event_type_(other.log_event_type_),
      status_code_(other.status_code_),
      start_(other.start_),
      pause_start_(other.pause_start_),
      pause_duration_(other.pause_duration_) {
  other.uploader_ = nullptr;
}

ScopedMetric& ScopedMetric::operator=(ScopedMetric&& other) {
  if (&other == this) {
    return *this;
  }

  uploader_ = other.uploader_;
  other.uploader_ = nullptr;
  log_event_type_ = other.log_event_type_;
  status_code_ = other.status_code_;
  start_ = other.start_;
  pause_start_ = other.pause_start_;
  pause_duration_ = other.pause_duration_;

  return *this;
}

void ScopedMetric::Pause() {
  if (pause_start_.has_value()) return;

  pause_start_ = std::chrono::steady_clock::now();
}

void ScopedMetric::Resume() {
  if (!pause_start_.has_value()) return;

  pause_duration_ += std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - pause_start_.value());
  pause_start_.reset();
}

}  // namespace orbit_metrics_uploader