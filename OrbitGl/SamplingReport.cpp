//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "SamplingReport.h"

#include "CallStackDataView.h"
#include "SamplingProfiler.h"
#include "SamplingReportDataView.h"

//-----------------------------------------------------------------------------
SamplingReport::SamplingReport(
    std::shared_ptr<class SamplingProfiler> a_SamplingProfiler) {
  m_Profiler = a_SamplingProfiler;
  m_SelectedAddress = 0;
  m_CallstackDataView = nullptr;
  m_SelectedSortedCallstackReport = nullptr;
  FillReport();
}

//-----------------------------------------------------------------------------
SamplingReport::~SamplingReport() { m_ThreadReports.clear(); }

//-----------------------------------------------------------------------------
void SamplingReport::FillReport() {
  const auto& sampleData = m_Profiler->GetThreadSampleData();

  for (ThreadSampleData* threadSampleData : sampleData) {
    ThreadID tid = threadSampleData->m_TID;

    if (tid == 0 && m_Profiler->GetGenerateSummary() == false) continue;

    std::shared_ptr<SamplingReportDataView> threadReport =
        std::make_shared<SamplingReportDataView>();
    threadReport->SetSampledFunctions(threadSampleData->m_SampleReport);
    threadReport->SetThreadID(tid);
    threadReport->SetSamplingProfiler(m_Profiler);
    threadReport->SetSamplingReport(this);
    m_ThreadReports.push_back(threadReport);
  }

  static int cnt = 0;
  std::string panelName = "samplingReport" + std::to_string(cnt++);
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
  assert(HasCallstacks());
  int maxIndex = (int)m_SelectedSortedCallstackReport->m_CallStacks.size() - 1;
  if (++m_SelectedAddressCallstackIndex > maxIndex) {
    m_SelectedAddressCallstackIndex = 0;
  }

  OnCallstackIndexChanged(m_SelectedAddressCallstackIndex);
}

//-----------------------------------------------------------------------------
void SamplingReport::DecrementCallstackIndex() {
  assert(HasCallstacks());
  int maxIndex = (int)m_SelectedSortedCallstackReport->m_CallStacks.size() - 1;
  if (--m_SelectedAddressCallstackIndex < 0) {
    m_SelectedAddressCallstackIndex = maxIndex;
  }

  OnCallstackIndexChanged(m_SelectedAddressCallstackIndex);
}

//-----------------------------------------------------------------------------
std::wstring SamplingReport::GetSelectedCallstackString() {
  if (m_SelectedSortedCallstackReport) {
    int numOccurances = m_SelectedSortedCallstackReport
                            ->m_CallStacks[m_SelectedAddressCallstackIndex]
                            .m_Count;
    int totalCallstacks = m_SelectedSortedCallstackReport->m_NumCallStacksTotal;

    return Format(
        L"%i of %i unique callstacks.  [%i/%i total callstacks](%.2f%%)",
        m_SelectedAddressCallstackIndex + 1,
        m_SelectedSortedCallstackReport->m_CallStacks.size(), numOccurances,
        totalCallstacks, 100.f * (float)numOccurances / (float)totalCallstacks);
  }

  return L"Callstacks";
}

//-----------------------------------------------------------------------------
void SamplingReport::OnCallstackIndexChanged(int a_Index) {
  if (a_Index >= 0 &&
      a_Index < (int)m_SelectedSortedCallstackReport->m_CallStacks.size()) {
    CallstackCount& cs = m_SelectedSortedCallstackReport->m_CallStacks[a_Index];
    m_SelectedAddressCallstackIndex = a_Index;
    m_CallstackDataView->SetCallStack(
        m_Profiler->GetCallStack(cs.m_CallstackId));
  } else {
    m_SelectedAddressCallstackIndex = 0;
  }
}
