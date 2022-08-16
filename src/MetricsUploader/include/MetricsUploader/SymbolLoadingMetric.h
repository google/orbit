// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef METRICS_UPLOADER_SYMBOL_LOADING_METRIC_H_
#define METRICS_UPLOADER_SYMBOL_LOADING_METRIC_H_

#include "MetricsUploader/MetricsUploader.h"
#include "MetricsUploader/orbit_log_event.pb.h"

namespace orbit_metrics_uploader {

class SymbolLoadingMetric {
 public:
  explicit SymbolLoadingMetric(MetricsUploader* uploader);
  SymbolLoadingMetric(const SymbolLoadingMetric& other) = delete;
  SymbolLoadingMetric& operator=(const SymbolLoadingMetric& other) = delete;
  SymbolLoadingMetric(SymbolLoadingMetric&& other);
  SymbolLoadingMetric& operator=(SymbolLoadingMetric&& other);
  ~SymbolLoadingMetric();

  void SetIsMainModule(bool is_main_module);
  void SetSymbolsFound(OrbitPerModuleSymbolLoadData::SymbolSource symbol_source,
                       OrbitPerModuleSymbolLoadData::SymbolFileSeparation symbol_file_separation);
  void SetSymbolsNotFound();
  void SetError();
  void SetCancelled();

 private:
  void Send();

  MetricsUploader* uploader_;
  std::chrono::steady_clock::time_point start_time_point_ = std::chrono::steady_clock::now();
  OrbitLogEvent::StatusCode status_code_ = OrbitLogEvent::SUCCESS;
  OrbitPerModuleSymbolLoadData proto_data_;
};

}  // namespace orbit_metrics_uploader

#endif  // METRICS_UPLOADER_SYMBOL_LOADING_METRIC_H_