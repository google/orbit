//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "OrbitType.h"
#include "ProcessUtils.h"
#include "SamplingProfiler.h"

class SamplingReportDataView : public DataView {
 public:
  SamplingReportDataView();

  const std::vector<std::wstring>& GetColumnHeaders() override;
  const std::vector<float>& GetColumnHeadersRatios() override;
  const std::vector<SortingOrder>& GetColumnInitialOrders() override;
  int GetDefaultSortingColumn() override;
  std::vector<std::wstring> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::wstring GetValue(int a_Row, int a_Column) override;
  const std::wstring& GetName() override { return m_Name; }

  void OnFilter(const std::wstring& a_Filter) override;
  void OnSort(int a_Column, std::optional<SortingOrder> a_NewOrder) override;
  void OnContextMenu(const std::wstring& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;
  void OnSelect(int a_Index) override;

  void LinkDataView(DataView* a_DataView) override;
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

  std::vector<SampledFunction> m_Functions;
  ThreadID m_TID;
  std::wstring m_Name;
  std::shared_ptr<SamplingProfiler> m_SamplingProfiler;
  class CallStackDataView* m_CallstackDataView;
  SamplingReport* m_SamplingReport;

  static void InitColumnsIfNeeded();
  static std::vector<std::wstring> s_Headers;
  static std::vector<int> s_HeaderMap;
  static std::vector<float> s_HeaderRatios;
  static std::vector<SortingOrder> s_InitialOrders;
};
