//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "ProcessUtils.h"

class ProcessesDataView : public DataView {
 public:
  ProcessesDataView();
  virtual const std::vector<std::wstring>& GetColumnHeaders() override;
  virtual const std::vector<float>& GetColumnHeadersRatios() override;
  virtual std::wstring GetValue(int a_Row, int a_Column) override;
  virtual std::wstring GetToolTip(int a_Row, int a_Column) override;
  virtual std::wstring GetLabel() override { return L"Processes"; }

  void OnFilter(const std::wstring& a_Filter) override;
  void OnSort(int a_Column, bool a_Toggle = true) override;
  void OnSelect(int a_Index) override;
  void OnTimer() override;
  void SetSelectedItem();
  bool SelectProcess(const std::wstring& a_ProcessName);
  std::shared_ptr<Process> SelectProcess(DWORD a_ProcessId);
  void UpdateProcessList();
  void SetRemoteProcessList(std::shared_ptr<ProcessList> a_RemoteProcessList);
  void SetRemoteProcess(std::shared_ptr<Process> a_Process);
  void SetModulesDataView(class ModulesDataView* a_ModulesCtrl) {
    m_ModulesDataView = a_ModulesCtrl;
  }
  void Refresh();
  void UpdateModuleDataView(std::shared_ptr<Process> a_Process);
  void SetIsRemote(bool a_Value) { m_IsRemote = a_Value; }

  std::shared_ptr<Process> GetSelectedProcess() const {
    return m_SelectedProcess;
  }

  // void SetSelectedProcessCtrl(SelectedProcessPanel* a_Panel ) {
  // m_SelectedProcessPanel = a_Panel; }

  enum PdvColumn {
    PDV_ProcessID,
    PDV_ProcessName,
    PDV_CPU,
    PDV_Type,
    PDV_NumColumns
  };

 protected:
  std::shared_ptr<Process> GetProcess(unsigned int a_Row) const;
  void ClearSelectedProcess();

  ProcessList m_ProcessList;
  std::shared_ptr<Process> m_RemoteProcess;
  ModulesDataView* m_ModulesDataView;
  std::shared_ptr<Process> m_SelectedProcess;
  static std::vector<float> s_HeaderRatios;
  bool m_IsRemote;
};
