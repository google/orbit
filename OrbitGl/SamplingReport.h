// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SAMPLING_REPORT_H_
#define ORBIT_GL_SAMPLING_REPORT_H_

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
  void UpdateReport();
  std::shared_ptr<SamplingProfiler> GetProfiler() const { return m_Profiler; }
  std::vector<SamplingReportDataView>& GetThreadReports() {
    return m_ThreadReports;
  }
  void SetCallstackDataView(class CallStackDataView* a_DataView) {
    m_CallstackDataView = a_DataView;
  }
  void OnSelectAddress(uint64_t a_Address, int32_t a_ThreadId);
  void OnCallstackIndexChanged(size_t a_Index);
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
  std::vector<SamplingReportDataView> m_ThreadReports;
  CallStackDataView* m_CallstackDataView;

  uint64_t m_SelectedAddress;
  int32_t m_SelectedTid;
  std::shared_ptr<SortedCallstackReport> m_SelectedSortedCallstackReport;
  size_t m_SelectedCallstackIndex;
  std::function<void()> m_UiRefreshFunc;
};

#endif  // ORBIT_GL_SAMPLING_REPORT_H_
