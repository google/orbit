// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIVE_FUNCTIONS_H_
#define LIVE_FUNCTIONS_H_

#include <cstdint>
#include <functional>

#include "LiveFunctionsDataView.h"
#include "MetricsUploader/MetricsUploader.h"
#include "OrbitBase/Profiling.h"
#include "TextBox.h"
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"

class OrbitApp;

class LiveFunctionsController {
 public:
  explicit LiveFunctionsController(OrbitApp* app,
                                   orbit_metrics_uploader::MetricsUploader* metrics_uploader)
      : live_functions_data_view_(this, app, metrics_uploader),
        app_{app},
        metrics_uploader_(metrics_uploader) {}

  LiveFunctionsDataView& GetDataView() { return live_functions_data_view_; }

  bool OnAllNextButton();
  bool OnAllPreviousButton();

  void OnNextButton(uint64_t id);
  void OnPreviousButton(uint64_t id);
  void OnDeleteButton(uint64_t id);

  void Reset();
  void OnDataChanged() { live_functions_data_view_.OnDataChanged(); }

  void SetAddIteratorCallback(
      std::function<void(uint64_t, const orbit_client_protos::FunctionInfo*)> callback) {
    add_iterator_callback_ = std::move(callback);
  }

  [[nodiscard]] uint64_t GetCaptureMin() const;
  [[nodiscard]] uint64_t GetCaptureMax() const;
  [[nodiscard]] uint64_t GetStartTime(uint64_t index) const;

  void AddIterator(uint64_t instrumented_function_id,
                   const orbit_client_protos::FunctionInfo* function);

 private:
  void Move();

  LiveFunctionsDataView live_functions_data_view_;

  absl::flat_hash_map<uint64_t, uint64_t> iterator_id_to_function_id_;
  absl::flat_hash_map<uint64_t, const TextBox*> current_textboxes_;

  std::function<void(uint64_t, const orbit_client_protos::FunctionInfo*)> add_iterator_callback_;

  uint64_t next_iterator_id_ = 1;
  uint64_t id_to_select_ = 0;

  OrbitApp* app_ = nullptr;
  orbit_metrics_uploader::MetricsUploader* metrics_uploader_;
};

#endif  // LIVE_FUNCTIONS_H_
