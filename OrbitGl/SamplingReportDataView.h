//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "CallStackDataView.h"
#include "DataView.h"
#include "OrbitType.h"
#include "ProcessUtils.h"
#include "SamplingProfiler.h"

class SamplingReportDataView : public DataView {
 public:
  SamplingReportDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return COLUMN_INCLUSIVE; }
  std::vector<std::string> GetContextMenu(
      int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) override;
  std::string GetValue(int a_Row, int a_Column) override;
  const std::string& GetName() { return m_Name; }

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

 protected:
  void DoSort() override;
  void DoFilter() override;
  const SampledFunction& GetSampledFunction(unsigned int a_Row) const;
  SampledFunction& GetSampledFunction(unsigned int a_Row);
  std::vector<Function*> GetFunctionsFromIndices(
      const std::vector<int>& a_Indices);
  std::vector<std::shared_ptr<Module>> GetModulesFromIndices(
      const std::vector<int>& a_Indices);

  std::vector<SampledFunction> m_Functions;
  ThreadID m_TID = -1;
  std::string m_Name;
  CallStackDataView* m_CallstackDataView;
  SamplingReport* m_SamplingReport = nullptr;

  enum COLUMN_INDEX {
    COLUMN_SELECTED,
    COLUMN_INDEX,
    COLUMN_FUNCTION_NAME,
    COLUMN_EXCLUSIVE,
    COLUMN_INCLUSIVE,
    COLUMN_MODULE_NAME,
    COLUMN_FILE,
    COLUMN_LINE,
    COLUMN_ADDRESS,
    COLUMN_NUM
  };

  static const std::string MENU_ACTION_SELECT;
  static const std::string MENU_ACTION_UNSELECT;
  static const std::string MENU_ACTION_MODULES_LOAD;
  static const std::string MENU_ACTION_DISASSEMBLY;
};
