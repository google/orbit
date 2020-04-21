//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "SamplingProfiler.h"
#include "SamplingReportDataView.h"

class SamplingReport {
 public:
  explicit SamplingReport(std::shared_ptr<SamplingProfiler> a_SamplingProfiler);

  void FillReport();
  std::shared_ptr<SamplingProfiler> GetProfiler() const { return m_Profiler; }
  const std::vector<std::shared_ptr<SamplingReportDataView>>&
  GetThreadReports() {
    return m_ThreadReports;
  }
  void SetCallstackDataView(class CallStackDataView* a_DataView) {
    m_CallstackDataView = a_DataView;
  }
  void OnSelectAddress(uint64_t a_Address, uint32_t a_ThreadId);
  void OnCallstackIndexChanged(int a_Index);
  void IncrementCallstackIndex();
  void DecrementCallstackIndex();
  std::string GetSelectedCallstackString();
  void SetUiRefreshFunc(std::function<void()> a_Func) {
    m_UiRefreshFunc = std::move(a_Func);
  }
  bool HasCallstacks() const {
    return m_SelectedSortedCallstackReport != nullptr;
  }

 protected:
  std::shared_ptr<SamplingProfiler> m_Profiler;
  std::vector<std::shared_ptr<SamplingReportDataView>> m_ThreadReports;
  CallStackDataView* m_CallstackDataView;

  uint64_t m_SelectedAddress;
  std::shared_ptr<SortedCallstackReport> m_SelectedSortedCallstackReport;
  int m_SelectedAddressCallstackIndex;
  std::function<void()> m_UiRefreshFunc;
};
