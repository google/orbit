//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "OrbitProcess.h"

class ProcessesDataView final : public DataView {
 public:
  ProcessesDataView();

  const std::vector<Column>& GetColumns() override;
  int GetDefaultSortingColumn() override { return COLUMN_CPU; }
  std::string GetValue(int row, int column) override;
  std::string GetToolTip(int row, int column) override;
  std::string GetLabel() override { return "Processes"; }

  void OnSelect(int index) override;
  bool SelectProcess(const std::string& process_name);
  std::shared_ptr<Process> SelectProcess(uint32_t process_id);
  void SetProcessList(
      const std::vector<std::shared_ptr<Process>>& process_list);
  void UpdateProcess(const std::shared_ptr<Process>& process);
  void SetModulesDataView(class ModulesDataView* modules_data_view) {
    modules_data_view_ = modules_data_view;
  }
  void UpdateModuleDataView(const std::shared_ptr<Process>& process);

 protected:
  void DoSort() override;
  void DoFilter() override;

 private:
  void UpdateProcessList();
  void SetSelectedItem();
  std::shared_ptr<Process> GetProcess(uint32_t row) const;
  void ClearSelectedProcess();

  std::vector<std::shared_ptr<Process>> process_list_;
  ModulesDataView* modules_data_view_ = nullptr;
  uint32_t selected_process_id_;

  enum ColumnIndex {
    COLUMN_PID,
    COLUMN_NAME,
    COLUMN_CPU,
    COLUMN_TYPE,
    COLUMN_NUM
  };
};
