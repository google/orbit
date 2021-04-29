// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MetricsUploader/ScopedMetric.h"

#include <chrono>

namespace orbit_metrics_uploader {

ScopedMetric::ScopedMetric(MetricsUploader* uploader, OrbitLogEvent_LogEventType log_event_type)
    : uploader_(uploader),
      log_event_type_(log_event_type),
      start_(std::chrono::steady_clock::now()) {}

ScopedMetric::~ScopedMetric() {
  if (uploader_ == nullptr) return;

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start_);
  uploader_->SendLogEvent(log_event_type_, duration, status_code_);
}

ScopedMetric::ScopedMetric(ScopedMetric&& other) noexcept
    : uploader_(other.uploader_),
      log_event_type_(other.log_event_type_),
      status_code_(other.status_code_),
      start_(other.start_) {
  other.uploader_ = nullptr;
}

ScopedMetric& ScopedMetric::operator=(ScopedMetric&& other) noexcept {
  if (&other == this) {
    return *this;
  }

  uploader_ = other.uploader_;
  other.uploader_ = nullptr;
  log_event_type_ = other.log_event_type_;
  status_code_ = other.status_code_;
  start_ = other.start_;

  return *this;
}

}  // namespace orbit_metrics_uploader