// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingReport.h"

#include "CallStackDataView.h"
#include "absl/strings/str_format.h"

SamplingReport::SamplingReport(std::shared_ptr<SamplingProfiler> sampling_profiler,
                               const CallstackData* callstack_data) {
  profiler_ = std::move(sampling_profiler);
  callstack_data_ = callstack_data;
  selected_address_ = 0;
  selected_thread_id_ = 0;
  callstack_data_view_ = nullptr;
  selected_sorted_callstack_report_ = nullptr;
  selected_callstack_index_ = 0;
  FillReport();
}

SamplingReport::~SamplingReport() { ClearReport(); }

void SamplingReport::ClearReport() {
  selected_sorted_callstack_report_ = nullptr;
  selected_callstack_index_ = 0;
  if (callstack_data_view_ != nullptr) {
    callstack_data_view_->ClearCallstack();
  }
}

void SamplingReport::FillReport() {
  const auto& sample_data = profiler_->GetThreadSampleData();

  for (ThreadSampleData* threadSampleData : sample_data) {
    ThreadID tid = threadSampleData->thread_id;

    if (tid == SamplingProfiler::kAllThreadsFakeTid && !profiler_->GetGenerateSummary()) continue;

    SamplingReportDataView thread_report;
    thread_report.SetSampledFunctions(threadSampleData->sampled_function);
    thread_report.SetThreadID(tid);
    thread_report.SetSamplingReport(this);
    thread_reports_.push_back(std::move(thread_report));
  }
}

void SamplingReport::UpdateReport() {
  if (callstack_data_ == nullptr) {
    return;
  }

  profiler_->ProcessSamples(*callstack_data_);
  for (SamplingReportDataView& thread_report : thread_reports_) {
    ThreadID thread_id = thread_report.GetThreadID();
    const ThreadSampleData* thread_sample_data =
        profiler_->GetThreadSampleDataByThreadId(thread_id);
    if (thread_sample_data != nullptr) {
      thread_report.SetSampledFunctions(thread_sample_data->sampled_function);
    }
  }

  // Refresh the displayed callstacks as they might not be up to date anymore,
  // for example the number of occurrences or of total callstacks might have
  // changed (OrbitSamplingReport::RefreshCallstackView will do the actual
  // update once OrbitApp::FireRefreshCallbacks is called).
  selected_sorted_callstack_report_ =
      profiler_->GetSortedCallstacksFromAddress(selected_address_, selected_thread_id_);
  if (selected_sorted_callstack_report_->callstacks_count.empty()) {
    ClearReport();
  } else {
    OnCallstackIndexChanged(selected_callstack_index_);
  }
}

void SamplingReport::OnSelectAddress(uint64_t address, ThreadID thread_id) {
  if (callstack_data_view_) {
    if (selected_address_ != address || selected_thread_id_ != thread_id) {
      selected_sorted_callstack_report_ =
          profiler_->GetSortedCallstacksFromAddress(address, thread_id);
      selected_address_ = address;
      selected_thread_id_ = thread_id;
      if (selected_sorted_callstack_report_->callstacks_count.empty()) {
        callstack_data_view_->ClearCallstack();
      } else {
        OnCallstackIndexChanged(0);
      }
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
    callstack_data_view_->SetCallStack(*callstack_data_->GetCallStack(cs.callstack_id));
  } else {
    selected_callstack_index_ = 0;
  }
}
