// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/SamplingReport.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/strings/str_format.h>

#include <algorithm>
#include <iterator>
#include <optional>

#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/OrbitApp.h"

using orbit_client_data::CallstackCount;
using orbit_client_data::CallstackInfo;
using orbit_client_data::CallstackType;
using orbit_client_data::PostProcessedSamplingData;
using orbit_client_data::ThreadID;
using orbit_client_data::ThreadSampleData;

SamplingReport::SamplingReport(
    OrbitApp* app, const orbit_client_data::CallstackData* callstack_data,
    const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data,
    bool has_summary)
    : app_{app},
      callstack_data_{callstack_data},
      post_processed_sampling_data_{post_processed_sampling_data},
      has_summary_{has_summary} {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_SCOPED_TIMED_LOG("SamplingReport::SamplingReport");
  ORBIT_CHECK(callstack_data_ != nullptr);
  ORBIT_CHECK(post_processed_sampling_data_ != nullptr);
  ORBIT_CHECK(app_ != nullptr);
  FillReport();
}

void SamplingReport::ClearReport() {
  selected_sorted_callstack_report_ = nullptr;
  selected_callstack_index_ = 0;
  if (callstack_data_view_ != nullptr) {
    callstack_data_view_->ClearCallstack();
  }
}

const orbit_client_data::CallstackData& SamplingReport::GetCallstackData() const {
  ORBIT_CHECK(callstack_data_ != nullptr);
  return *callstack_data_;
}

[[nodiscard]] std::optional<absl::flat_hash_set<uint64_t>> SamplingReport::GetSelectedCallstackIds()
    const {
  if (!selected_sorted_callstack_report_) return std::nullopt;

  absl::flat_hash_set<uint64_t> result;
  const std::vector<CallstackCount>& selected_callstacks =
      selected_sorted_callstack_report_->callstack_counts;
  std::transform(std::begin(selected_callstacks), std::end(selected_callstacks),
                 std::inserter(result, result.begin()), [](const CallstackCount& callstack_count) {
                   return callstack_count.callstack_id;
                 });

  return result;
}

void SamplingReport::FillReport() {
  const std::vector<const ThreadSampleData*>& sample_data =
      post_processed_sampling_data_->GetSortedThreadSampleData();

  for (const ThreadSampleData* thread_sample_data : sample_data) {
    orbit_data_views::SamplingReportDataView thread_report{app_};
    thread_report.Init();
    thread_report.SetSampledFunctions(thread_sample_data->sampled_functions);
    thread_report.SetThreadID(thread_sample_data->thread_id);
    thread_report.SetStackEventsCount(thread_sample_data->samples_count);
    thread_report.SetSamplingReport(this);
    thread_data_views_.push_back(std::move(thread_report));
  }
}

void SamplingReport::UpdateDisplayedCallstack() {
  if (selected_addresses_.empty()) {
    ClearReport();
    return;
  }

  selected_sorted_callstack_report_ =
      post_processed_sampling_data_->GetSortedCallstackReportFromFunctionAddresses(
          std::vector<uint64_t>(selected_addresses_.begin(), selected_addresses_.end()),
          selected_thread_id_);
  if (selected_sorted_callstack_report_->callstack_counts.empty()) {
    ClearReport();
  } else {
    OnCallstackIndexChanged(selected_callstack_index_);
  }
}

void SamplingReport::UpdateReport(
    const orbit_client_data::CallstackData* callstack_data,
    const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data) {
  callstack_data_ = callstack_data;
  post_processed_sampling_data_ = post_processed_sampling_data;

  for (orbit_data_views::SamplingReportDataView& thread_report : thread_data_views_) {
    ThreadID thread_id = thread_report.GetThreadID();
    const ThreadSampleData* thread_sample_data =
        post_processed_sampling_data_->GetThreadSampleDataByThreadId(thread_id);
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
      selected_callstack_index_ = 0;
      UpdateDisplayedCallstack();
    }
  }

  if (ui_refresh_func_) {
    ui_refresh_func_();
  }
}

void SamplingReport::IncrementCallstackIndex() {
  ORBIT_CHECK(HasCallstacks());
  size_t max_index = selected_sorted_callstack_report_->callstack_counts.size() - 1;
  if (++selected_callstack_index_ > max_index) {
    selected_callstack_index_ = 0;
  }

  OnCallstackIndexChanged(selected_callstack_index_);
}

void SamplingReport::DecrementCallstackIndex() {
  ORBIT_CHECK(HasCallstacks());
  size_t max_index = selected_sorted_callstack_report_->callstack_counts.size() - 1;
  if (selected_callstack_index_ == 0) {
    selected_callstack_index_ = max_index;
  } else {
    --selected_callstack_index_;
  }

  OnCallstackIndexChanged(selected_callstack_index_);
}

std::string SamplingReport::GetSelectedCallstackString() const {
  if (selected_sorted_callstack_report_ == nullptr) {
    return "Callstacks";
  }

  int num_occurrences =
      selected_sorted_callstack_report_->callstack_counts[selected_callstack_index_].count;
  int total_callstacks = selected_sorted_callstack_report_->total_callstack_count;

  uint64_t callstack_id =
      selected_sorted_callstack_report_->callstack_counts[selected_callstack_index_].callstack_id;
  const CallstackInfo* callstack = callstack_data_->GetCallstack(callstack_id);
  ORBIT_CHECK(callstack != nullptr);
  CallstackType callstack_type = callstack->type();

  std::string type_string =
      (callstack_type == CallstackType::kComplete)
          ? ""
          : absl::StrFormat(
                "  -  <span style=\" color:#ff8000; font-weight: bold;\">Unwind error</span><span "
                "style=\" color:#ff8000;\"> (%s)</span>",
                orbit_client_data::CallstackTypeToString(callstack_type));
  return absl::StrFormat(
      "%i of %i unique callstacks  [%i/%i total samples] (%.2f%%)%s", selected_callstack_index_ + 1,
      selected_sorted_callstack_report_->callstack_counts.size(), num_occurrences, total_callstacks,
      100.f * num_occurrences / total_callstacks, type_string);
}

double SamplingReport::ComputeUnwindErrorRatio(uint32_t thread_id) const {
  if (post_processed_sampling_data_ == nullptr) {
    return 0.;
  }
  const auto* thread_sampling_data =
      post_processed_sampling_data_->GetThreadSampleDataByThreadId(thread_id);
  if (thread_sampling_data == nullptr) {
    return 0.;
  }
  return static_cast<double>(thread_sampling_data->unwinding_errors_count) /
         thread_sampling_data->samples_count;
}

std::string SamplingReport::GetSelectedCallstackTooltipString() const {
  if (selected_sorted_callstack_report_ == nullptr) {
    return "";
  }

  uint64_t callstack_id =
      selected_sorted_callstack_report_->callstack_counts[selected_callstack_index_].callstack_id;
  const CallstackInfo* callstack = callstack_data_->GetCallstack(callstack_id);
  ORBIT_CHECK(callstack != nullptr);
  CallstackType callstack_type = callstack->type();

  if (callstack_type == CallstackType::kComplete) {
    return "";
  }

  return absl::StrFormat("The callstack could not be unwound successfully.<br/>%s",
                         orbit_client_data::CallstackTypeToDescription(callstack_type));
}

void SamplingReport::OnCallstackIndexChanged(size_t index) {
  if (index < selected_sorted_callstack_report_->callstack_counts.size()) {
    const CallstackCount& callstack_count =
        selected_sorted_callstack_report_->callstack_counts[index];
    selected_callstack_index_ = index;
    const CallstackInfo* callstack = callstack_data_->GetCallstack(callstack_count.callstack_id);
    ORBIT_CHECK(callstack != nullptr);
    callstack_data_view_->SetCallstack(*callstack);
    callstack_data_view_->SetFunctionsToHighlight(selected_addresses_);
  } else {
    selected_callstack_index_ = 0;
  }
}
