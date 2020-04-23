//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "ProcessUtils.h"

class ProcessesDataView : public DataView {
 public:
  ProcessesDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return COLUMN_CPU; }
  std::string GetValue(int a_Row, int a_Column) override;
  std::string GetToolTip(int a_Row, int a_Column) override;
  std::string GetLabel() override { return "Processes"; }

  void OnSelect(int a_Index) override;
  void OnTimer() override;
  void SetSelectedItem();
  bool SelectProcess(const std::string& a_ProcessName);
  std::shared_ptr<Process> SelectProcess(DWORD a_ProcessId);
  void UpdateProcessList();
  void SetRemoteProcessList(ProcessList a_RemoteProcessList);
  void SetRemoteProcess(const std::shared_ptr<Process>& a_Process);
  void SetModulesDataView(class ModulesDataView* a_ModulesCtrl) {
    m_ModulesDataView = a_ModulesCtrl;
  }
  void Refresh();
  void UpdateModuleDataView(const std::shared_ptr<Process>& a_Process);
  void SetIsRemote(bool a_Value) { m_IsRemote = a_Value; }

 protected:
  void DoSort() override;
  void DoFilter() override;
  std::shared_ptr<Process> GetProcess(unsigned int a_Row) const;
  void ClearSelectedProcess();

  ProcessList m_ProcessList;
  std::shared_ptr<Process> m_RemoteProcess;
  ModulesDataView* m_ModulesDataView = nullptr;
  std::shared_ptr<Process> m_SelectedProcess;
  bool m_IsRemote;

  enum ColumnIndex {
    COLUMN_PID,
    COLUMN_NAME,
    COLUMN_CPU,
    COLUMN_TYPE,
    COLUMN_NUM
  };
};
