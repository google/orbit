//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataView.h"
#include "OrbitProcess.h"

class ProcessesDataView final : public DataView {
 public:
  ProcessesDataView();

  void SetSelectionListener(
      const std::function<void(uint32_t)>& selection_listener);
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
  uint32_t GetSelectedProcessId() const;

 protected:
  void DoSort() override;
  void DoFilter() override;

 private:
  void UpdateProcessList();
  void SetSelectedItem();
  std::shared_ptr<Process> GetProcess(uint32_t row) const;

  std::vector<std::shared_ptr<Process>> process_list_;
  uint32_t selected_process_id_;

  std::function<void(uint32_t)> selection_listener_;

  enum ColumnIndex {
    COLUMN_PID,
    COLUMN_NAME,
    COLUMN_CPU,
    COLUMN_TYPE,
    COLUMN_NUM
  };
};
