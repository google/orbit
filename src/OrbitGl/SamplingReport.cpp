// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingReport.h"

#include <absl/meta/type_traits.h>

#include <algorithm>

#include "App.h"
#include "CallStackDataView.h"
#include "OrbitBase/Logging.h"
#include "absl/strings/str_format.h"

using orbit_client_data::CallStack;
using orbit_client_data::CallstackCount;
using orbit_client_data::PostProcessedSamplingData;
using orbit_client_data::ThreadID;
using orbit_client_data::ThreadSampleData;

SamplingReport::SamplingReport(
    OrbitApp* app, PostProcessedSamplingData post_processed_sampling_data,
    absl::flat_hash_map<uint64_t, std::shared_ptr<CallStack>> unique_callstacks, bool has_summary)
    : post_processed_sampling_data_{std::move(post_processed_sampling_data)},
      unique_callstacks_{std::move(unique_callstacks)},
      has_summary_{has_summary},
      app_{app} {
  selected_thread_id_ = 0;
  callstack_data_view_ = nullptr;
  selected_sorted_callstack_report_ = nullptr;
  selected_callstack_index_ = 0;
  FillReport();
}

void SamplingReport::ClearReport() {
  selected_sorted_callstack_report_ = nullptr;
  selected_callstack_index_ = 0;
  if (callstack_data_view_ != nullptr) {
    callstack_data_view_->ClearCallstack();
  }
}

void SamplingReport::FillReport() {
  const auto& sample_data = post_processed_sampling_data_.GetThreadSampleData();

  for (const ThreadSampleData& thread_sample_data : sample_data) {
    SamplingReportDataView thread_report{app_};
    thread_report.SetSampledFunctions(thread_sample_data.sampled_functions);
    thread_report.SetThreadID(thread_sample_data.thread_id);
    thread_report.SetSamplingReport(this);
    thread_reports_.push_back(std::move(thread_report));
  }
}

void SamplingReport::UpdateDisplayedCallstack() {
  if (selected_addresses_.empty()) {
    ClearReport();
    return;
  }

  selected_sorted_callstack_report_ =
      post_processed_sampling_data_.GetSortedCallstackReportFromAddresses(
          std::vector<uint64_t>(selected_addresses_.begin(), selected_addresses_.end()),
          selected_thread_id_);
  if (selected_sorted_callstack_report_->callstacks_count.empty()) {
    ClearReport();
  } else {
    OnCallstackIndexChanged(selected_callstack_index_);
  }
}

void SamplingReport::UpdateReport(
    PostProcessedSamplingData post_processed_sampling_data,
    absl::flat_hash_map<uint64_t, std::shared_ptr<CallStack>> unique_callstacks) {
  unique_callstacks_ = std::move(unique_callstacks);
  post_processed_sampling_data_ = std::move(post_processed_sampling_data);

  for (SamplingReportDataView& thread_report : thread_reports_) {
    ThreadID thread_id = thread_report.GetThreadID();
    const ThreadSampleData* thread_sample_data =
        post_processed_sampling_data_.GetThreadSampleDataByThreadId(thread_id);
    if (thread_sample_data != nullptr) {
      thread_report.SetSampledFunctions(thread_sample_data->sampled_functions);
    }
  }

  // Refresh the displayed callstacks as they might not be up to date anymore,
  // for example the number of occurrences or of total callstacks might have
  // changed (OrbitSamplingReport::RefreshCallstackView will do the actual
  // update once OrbitApp::FireRefreshCallbacks is called).
  UpdateDisplayedCallstack();
}

void SamplingReport::OnSelectAddresses(const absl::flat_hash_set<uint64_t>& addresses,
                                       ThreadID thread_id) {
  if (callstack_data_view_ != nullptr) {
    if (selected_addresses_ != addresses || selected_thread_id_ != thread_id) {
      selected_addresses_ = addresses;
      selected_thread_id_ = thread_id;
      UpdateDisplayedCallstack();
    }
  }

  if (ui_refresh_func_) {
    ui_refresh_func_();
  }
}

void SamplingReport::IncrementCallstackIndex() {
  CHECK(HasCallstacks());
  size_t max_index = selected_sorted_callstack_report_->callstacks_count.size() - 1;
  if (++selected_callstack_index_ > max_index) {
    selected_callstack_index_ = 0;
  }

  OnCallstackIndexChanged(selected_callstack_index_);
}

void SamplingReport::DecrementCallstackIndex() {
  CHECK(HasCallstacks());
  size_t max_index = selected_sorted_callstack_report_->callstacks_count.size() - 1;
  if (selected_callstack_index_ == 0) {
    selected_callstack_index_ = max_index;
  } else {
    --selected_callstack_index_;
  }

  OnCallstackIndexChanged(selected_callstack_index_);
}

std::string SamplingReport::GetSelectedCallstackString() const {
  if (selected_sorted_callstack_report_) {
    int num_occurrences =
        selected_sorted_callstack_report_->callstacks_count[selected_callstack_index_].count;
    int total_callstacks = selected_sorted_callstack_report_->callstacks_total_count;

    return absl::StrFormat(
        "%i of %i unique callstacks.  [%i/%i total callstacks](%.2f%%)",
        selected_callstack_index_ + 1, selected_sorted_callstack_report_->callstacks_count.size(),
        num_occurrences, total_callstacks, 100.f * num_occurrences / total_callstacks);
  }

  return "Callstacks";
}

void SamplingReport::OnCallstackIndexChanged(size_t index) {
  if (index < selected_sorted_callstack_report_->callstacks_count.size()) {
    const CallstackCount& cs = selected_sorted_callstack_report_->callstacks_count[index];
    selected_callstack_index_ = index;
    auto it = unique_callstacks_.find(cs.callstack_id);
    CHECK(it != unique_callstacks_.end());
    callstack_data_view_->SetCallStack(*it->second);
    callstack_data_view_->SetFunctionsToHighlight(selected_addresses_);
  } else {
    selected_callstack_index_ = 0;
  }
}
