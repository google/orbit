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

  const std::vector<std::string>& GetColumnHeaders() override;
  const std::vector<float>& GetColumnHeadersRatios() override;
  const std::vector<SortingOrder>& GetColumnInitialOrders() override;
  int GetDefaultSortingColumn() override;
  std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::string GetValue(int a_Row, int a_Column) override;
  const std::string& GetName() override { return m_Name; }

  void OnFilter(const std::string& a_Filter) override;
  void OnSort(int a_Column, std::optional<SortingOrder> a_NewOrder) override;
  void OnContextMenu(const std::string& a_Action, int a_MenuIndex,
                     const std::vector<int>& a_ItemIndices) override;
  void OnSelect(int a_Index) override;

  void LinkDataView(DataView* a_DataView) override;
  void SetSamplingReport(class SamplingReport* a_SamplingReport) {
    m_SamplingReport = a_SamplingReport;
  }
  void SetSampledFunctions(const std::vector<SampledFunction>& a_Functions);
  void SetThreadID(ThreadID a_TID);
  std::vector<SampledFunction>& GetSampledFunctions() { return m_Functions; }

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
  const SampledFunction& GetSampledFunction(unsigned int a_Row) const;
  SampledFunction& GetSampledFunction(unsigned int a_Row);
  std::vector<Function*> GetFunctionsFromIndices(
      const std::vector<int>& a_Indices);
  std::vector<std::shared_ptr<Module>> GetModulesFromIndices(
      const std::vector<int>& a_Indices);

  std::vector<SampledFunction> m_Functions;
  ThreadID m_TID = -1;
  std::string m_Name;
  class CallStackDataView* m_CallstackDataView;
  SamplingReport* m_SamplingReport = nullptr;

  static void InitColumnsIfNeeded();
  static std::vector<std::string> s_Headers;
  static std::vector<int> s_HeaderMap;
  static std::vector<float> s_HeaderRatios;
  static std::vector<SortingOrder> s_InitialOrders;

  static const std::string MENU_ACTION_SELECT;
  static const std::string MENU_ACTION_UNSELECT;
  static const std::string MENU_ACTION_MODULES_LOAD;
  static const std::string MENU_ACTION_DISASSEMBLY;
};
