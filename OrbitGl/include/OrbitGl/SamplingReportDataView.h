//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "OrbitCore/OrbitType.h"
#include "OrbitCore/ProcessUtils.h"
#include "OrbitCore/SamplingProfiler.h"

class SamplingReportDataView : public DataView {
 public:
  SamplingReportDataView();

  virtual const std::vector<std::wstring>& GetColumnHeaders() override;
  virtual const std::vector<float>& GetColumnHeadersRatios() override;
  virtual std::vector<std::wstring> GetContextMenu(int a_Index) override;
  virtual std::wstring GetValue(int a_Row, int a_Column) override;
  virtual const std::wstring& GetName() override { return m_Name; }

  void OnFilter(const std::wstring& a_Filter) override;
  void OnSort(int a_Column, bool a_Toggle = true) override;
  void OnContextMenu(const std::wstring& a_Action, int a_MenuIndex,
                     std::vector<int>& a_ItemIndices) override;
  void OnSelect(int a_Index) override;

  virtual void LinkDataView(DataView* a_DataView) override;
  void SetSamplingProfiler(std::shared_ptr<SamplingProfiler>& a_Profiler) {
    m_SamplingProfiler = a_Profiler;
  }
  void SetSamplingReport(class SamplingReport* a_SamplingReport) {
    m_SamplingReport = a_SamplingReport;
  }
  void SetSampledFunctions(const std::vector<SampledFunction>& a_Functions);
  void SetThreadID(ThreadID a_TID);
  std::vector<SampledFunction>& GetFunctions() { return m_Functions; }

  enum SamplingColumn {
    Toggle,
    Index,
    FunctionName,
    Exclusive,
    Inclusive,
    ModuleName,
    SourceFile,
    SourceLine,
    Address,
    NumColumns
  };

 protected:
  const SampledFunction& GetFunction(unsigned int a_Row) const;
  SampledFunction& GetFunction(unsigned int a_Row);

 protected:
  std::vector<SampledFunction> m_Functions;
  ThreadID m_TID;
  std::wstring m_Name;
  std::shared_ptr<SamplingProfiler> m_SamplingProfiler;
  class CallStackDataView* m_CallstackDataView;
  SamplingReport* m_SamplingReport;

  static std::vector<int> s_HeaderMap;
  static std::vector<float> s_HeaderRatios;
};
