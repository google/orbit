// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SAMPLING_REPORT_H_
#define ORBIT_GL_SAMPLING_REPORT_H_

#include <absl/container/flat_hash_map.h>
#include <stddef.h>

#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackTypes.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientProtos/capture_data.pb.h"
#include "DataViews/CallstackDataView.h"
#include "DataViews/SamplingReportDataView.h"
#include "DataViews/SamplingReportInterface.h"

class OrbitApp;

class SamplingReport : public orbit_data_views::SamplingReportInterface {
 public:
  explicit SamplingReport(
      OrbitApp* app, orbit_client_data::PostProcessedSamplingData post_processed_sampling_data,
      absl::flat_hash_map<uint64_t, std::shared_ptr<orbit_client_protos::CallstackInfo>>
          unique_callstacks,
      bool has_summary = true);
  void UpdateReport(
      orbit_client_data::PostProcessedSamplingData post_processed_sampling_data,
      absl::flat_hash_map<uint64_t, std::shared_ptr<orbit_client_protos::CallstackInfo>>
          unique_callstacks);
  [[nodiscard]] std::vector<orbit_data_views::SamplingReportDataView>& GetThreadReports() {
    return thread_reports_;
  };
  void SetCallstackDataView(orbit_data_views::CallstackDataView* data_view) override {
    callstack_data_view_ = data_view;
  };
  void OnSelectAddresses(const absl::flat_hash_set<uint64_t>& addresses,
                         orbit_client_data::ThreadID thread_id) override;
  void IncrementCallstackIndex();
  void DecrementCallstackIndex();
  [[nodiscard]] std::string GetSelectedCallstackString() const;
  void SetUiRefreshFunc(std::function<void()> func) { ui_refresh_func_ = std::move(func); };
  [[nodiscard]] bool HasCallstacks() const { return selected_sorted_callstack_report_ != nullptr; };
  [[nodiscard]] bool HasSamples() const { return !thread_reports_.empty(); }
  [[nodiscard]] bool has_summary() const { return has_summary_; }
  void ClearReport();

 private:
  void FillReport();
  void OnCallstackIndexChanged(size_t index);
  void UpdateDisplayedCallstack();

  orbit_client_data::PostProcessedSamplingData post_processed_sampling_data_;
  absl::flat_hash_map<uint64_t, std::shared_ptr<orbit_client_protos::CallstackInfo>>
      unique_callstacks_;
  std::vector<orbit_data_views::SamplingReportDataView> thread_reports_;
  orbit_data_views::CallstackDataView* callstack_data_view_ = nullptr;

  absl::flat_hash_set<uint64_t> selected_addresses_;
  orbit_client_data::ThreadID selected_thread_id_ = 0;
  std::unique_ptr<orbit_client_data::SortedCallstackReport> selected_sorted_callstack_report_ =
      nullptr;
  size_t selected_callstack_index_ = 0;
  std::function<void()> ui_refresh_func_;
  bool has_summary_;
  OrbitApp* app_ = nullptr;
};

#endif  // ORBIT_GL_SAMPLING_REPORT_H_
