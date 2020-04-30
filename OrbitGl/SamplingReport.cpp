//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

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
  m_CallstackDataView = nullptr;
  m_SelectedSortedCallstackReport = nullptr;
  m_SelectedAddressCallstackIndex = 0;
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

//-----------------------------------------------------------------------------
void SamplingReport::OnSelectAddress(uint64_t a_Address, ThreadID a_ThreadId) {
  if (m_CallstackDataView) {
    if (m_SelectedAddress != a_Address) {
      m_SelectedSortedCallstackReport =
          m_Profiler->GetSortedCallstacksFromAddress(a_Address, a_ThreadId);
      m_SelectedAddress = a_Address;
      OnCallstackIndexChanged(0);
    }
  }

  if (m_UiRefreshFunc) {
    m_UiRefreshFunc();
  }
}

//-----------------------------------------------------------------------------
void SamplingReport::IncrementCallstackIndex() {
  DCHECK(HasCallstacks());
  size_t maxIndex = m_SelectedSortedCallstackReport->m_CallStacks.size() - 1;
  if (++m_SelectedAddressCallstackIndex > maxIndex) {
    m_SelectedAddressCallstackIndex = 0;
  }

  OnCallstackIndexChanged(m_SelectedAddressCallstackIndex);
}

//-----------------------------------------------------------------------------
void SamplingReport::DecrementCallstackIndex() {
  assert(HasCallstacks());
  size_t maxIndex = m_SelectedSortedCallstackReport->m_CallStacks.size() - 1;
  if (m_SelectedAddressCallstackIndex == 0) {
    m_SelectedAddressCallstackIndex = maxIndex;
  }

  OnCallstackIndexChanged(m_SelectedAddressCallstackIndex);
}

//-----------------------------------------------------------------------------
std::string SamplingReport::GetSelectedCallstackString() {
  if (m_SelectedSortedCallstackReport) {
    int numOccurances = m_SelectedSortedCallstackReport
                            ->m_CallStacks[m_SelectedAddressCallstackIndex]
                            .m_Count;
    int totalCallstacks = m_SelectedSortedCallstackReport->m_NumCallStacksTotal;

    return absl::StrFormat(
        "%i of %i unique callstacks.  [%i/%i total callstacks](%.2f%%)",
        m_SelectedAddressCallstackIndex + 1,
        m_SelectedSortedCallstackReport->m_CallStacks.size(), numOccurances,
        totalCallstacks, 100.f * numOccurances / totalCallstacks);
  }

  return "Callstacks";
}

//-----------------------------------------------------------------------------
void SamplingReport::OnCallstackIndexChanged(size_t a_Index) {
  if (a_Index < m_SelectedSortedCallstackReport->m_CallStacks.size()) {
    CallstackCount& cs = m_SelectedSortedCallstackReport->m_CallStacks[a_Index];
    m_SelectedAddressCallstackIndex = a_Index;
    m_CallstackDataView->SetCallStack(
        m_Profiler->GetCallStack(cs.m_CallstackId));
  } else {
    m_SelectedAddressCallstackIndex = 0;
  }
}
