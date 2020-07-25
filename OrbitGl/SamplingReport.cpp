// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingReport.h"

#include <utility>

#include "CallStackDataView.h"
#include "SamplingProfiler.h"
#include "SamplingReportDataView.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
SamplingReport::SamplingReport(
    std::shared_ptr<class SamplingProfiler> a_SamplingProfiler) {
  m_Profiler = std::move(a_SamplingProfiler);
  m_SelectedAddress = 0;
  m_SelectedTid = 0;
  m_CallstackDataView = nullptr;
  m_SelectedSortedCallstackReport = nullptr;
  m_SelectedCallstackIndex = 0;
  FillReport();
}

//-----------------------------------------------------------------------------
void SamplingReport::FillReport() {
  const auto& sampleData = m_Profiler->GetThreadSampleData();

  for (ThreadSampleData* threadSampleData : sampleData) {
    ThreadID tid = threadSampleData->m_TID;

    if (tid == 0 && !m_Profiler->GetGenerateSummary()) continue;

    SamplingReportDataView threadReport;
    threadReport.SetSampledFunctions(threadSampleData->m_SampleReport);
    threadReport.SetThreadID(tid);
    threadReport.SetSamplingReport(this);
    m_ThreadReports.push_back(std::move(threadReport));
  }
}

void SamplingReport::UpdateReport() {
  m_Profiler->ProcessSamples();
  for (SamplingReportDataView& thread_report : m_ThreadReports) {
    int32_t thread_id = thread_report.GetThreadID();
    const ThreadSampleData* thread_sample_data =
        m_Profiler->GetThreadSampleDataByThreadId(thread_id);
    if (thread_sample_data != nullptr) {
      thread_report.SetSampledFunctions(thread_sample_data->m_SampleReport);
    }
  }

  // Refresh the displayed callstacks as they might not be up to date anymore,
  // for example the number of occurrences or of total callstacks might have
  // changed (OrbitSamplingReport::RefreshCallstackView will do the actual
  // update once OrbitApp::FireRefreshCallbacks is called).
  m_SelectedSortedCallstackReport = m_Profiler->GetSortedCallstacksFromAddress(
      m_SelectedAddress, m_SelectedTid);
  if (m_SelectedSortedCallstackReport->m_CallStacks.empty()) {
    m_SelectedSortedCallstackReport = nullptr;
    m_SelectedCallstackIndex = 0;
    m_CallstackDataView->SetCallStack(nullptr);
  } else {
    OnCallstackIndexChanged(m_SelectedCallstackIndex);
  }
}

//-----------------------------------------------------------------------------
void SamplingReport::OnSelectAddress(uint64_t a_Address, int32_t a_ThreadId) {
  if (m_CallstackDataView) {
    if (m_SelectedAddress != a_Address || m_SelectedTid != a_ThreadId) {
      m_SelectedSortedCallstackReport =
          m_Profiler->GetSortedCallstacksFromAddress(a_Address, a_ThreadId);
      m_SelectedAddress = a_Address;
      m_SelectedTid = a_ThreadId;
      OnCallstackIndexChanged(0);
    }
  }

  if (m_UiRefreshFunc) {
    m_UiRefreshFunc();
  }
}

//-----------------------------------------------------------------------------
void SamplingReport::IncrementCallstackIndex() {
  CHECK(HasCallstacks());
  size_t maxIndex = m_SelectedSortedCallstackReport->m_CallStacks.size() - 1;
  if (++m_SelectedCallstackIndex > maxIndex) {
    m_SelectedCallstackIndex = 0;
  }

  OnCallstackIndexChanged(m_SelectedCallstackIndex);
}

//-----------------------------------------------------------------------------
void SamplingReport::DecrementCallstackIndex() {
  CHECK(HasCallstacks());
  size_t maxIndex = m_SelectedSortedCallstackReport->m_CallStacks.size() - 1;
  if (m_SelectedCallstackIndex == 0) {
    m_SelectedCallstackIndex = maxIndex;
  } else {
    --m_SelectedCallstackIndex;
  }

  OnCallstackIndexChanged(m_SelectedCallstackIndex);
}

//-----------------------------------------------------------------------------
std::string SamplingReport::GetSelectedCallstackString() {
  if (m_SelectedSortedCallstackReport) {
    int num_occurrences =
        m_SelectedSortedCallstackReport->m_CallStacks[m_SelectedCallstackIndex]
            .m_Count;
    int total_callstacks =
        m_SelectedSortedCallstackReport->m_NumCallStacksTotal;

    return absl::StrFormat(
        "%i of %i unique callstacks.  [%i/%i total callstacks](%.2f%%)",
        m_SelectedCallstackIndex + 1,
        m_SelectedSortedCallstackReport->m_CallStacks.size(), num_occurrences,
        total_callstacks, 100.f * num_occurrences / total_callstacks);
  }

  return "Callstacks";
}

//-----------------------------------------------------------------------------
void SamplingReport::OnCallstackIndexChanged(size_t a_Index) {
  if (a_Index < m_SelectedSortedCallstackReport->m_CallStacks.size()) {
    CallstackCount& cs = m_SelectedSortedCallstackReport->m_CallStacks[a_Index];
    m_SelectedCallstackIndex = a_Index;
    m_CallstackDataView->SetCallStack(
        m_Profiler->GetCallstack(cs.m_CallstackId));
  } else {
    m_SelectedCallstackIndex = 0;
  }
}
