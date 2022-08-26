// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MetricsUploader/SymbolLoadingMetric.h"

#include <chrono>

#include "MetricsUploader/orbit_log_event.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_metrics_uploader {

SymbolLoadingMetric::SymbolLoadingMetric(MetricsUploader* uploader) : uploader_(uploader) {
  ORBIT_CHECK(uploader_ != nullptr);
}

SymbolLoadingMetric::~SymbolLoadingMetric() {
  if (uploader_ == nullptr) return;

  Send();
}

SymbolLoadingMetric::SymbolLoadingMetric(SymbolLoadingMetric&& other)
    : uploader_(other.uploader_),
      start_time_point_(other.start_time_point_),
      status_code_(other.status_code_),
      proto_data_(other.proto_data_) {
  other.uploader_ = nullptr;
}

SymbolLoadingMetric& SymbolLoadingMetric::operator=(SymbolLoadingMetric&& other) {
  if (&other == this) {
    return *this;
  }

  uploader_ = other.uploader_;
  other.uploader_ = nullptr;
  start_time_point_ = other.start_time_point_;
  status_code_ = other.status_code_;
  proto_data_ = other.proto_data_;

  return *this;
}

void SymbolLoadingMetric::Send() {
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start_time_point_);

  uploader_->SendSymbolLoadEvent(proto_data_, duration, status_code_);
}

void SymbolLoadingMetric::SetIsMainModule(bool is_main_module) {
  proto_data_.set_is_main_module(is_main_module
                                     ? OrbitPerModuleSymbolLoadData::IS_MAIN_MODULE_TRUE
                                     : OrbitPerModuleSymbolLoadData::IS_MAIN_MODULE_FALSE);
}

void SymbolLoadingMetric::SetSymbolsNotFound() {
  proto_data_.set_symbols_found(OrbitPerModuleSymbolLoadData::SYMBOLS_FOUND_FALSE);
}

void SymbolLoadingMetric::SetSymbolsFound(
    OrbitPerModuleSymbolLoadData::SymbolSource symbol_source,
    OrbitPerModuleSymbolLoadData::SymbolFileSeparation symbol_file_separation) {
  proto_data_.set_symbols_found(OrbitPerModuleSymbolLoadData::SYMBOLS_FOUND_TRUE);
  proto_data_.set_symbol_source(symbol_source);
  proto_data_.set_symbol_file_separation(symbol_file_separation);
}

void SymbolLoadingMetric::SetError() { status_code_ = OrbitLogEvent::INTERNAL_ERROR; }

void SymbolLoadingMetric::SetCancelled() { status_code_ = OrbitLogEvent::CANCELLED; }

}  // namespace orbit_metrics_uploader