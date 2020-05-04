//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ProcessesDataView.h"

#include <utility>

#include "App.h"
#include "Callstack.h"
#include "Capture.h"
#include "ModulesDataView.h"
#include "OrbitType.h"
#include "Params.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
ProcessesDataView::ProcessesDataView() : DataView(DataViewType::PROCESSES) {
}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& ProcessesDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_PID] = {"PID", .0f, SortingOrder::Ascending};
    columns[COLUMN_NAME] = {"Name", .5f, SortingOrder::Ascending};
    columns[COLUMN_CPU] = {"CPU", .0f, SortingOrder::Descending};
    columns[COLUMN_TYPE] = {"Type", .0f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string ProcessesDataView::GetValue(int row, int col) {
  const Process& process = *GetProcess(row);

  switch (col) {
    case COLUMN_PID:
      return std::to_string(process.GetID());
    case COLUMN_NAME:
      return process.GetName() + (process.IsElevated() ? "*" : "");
    case COLUMN_CPU:
      return absl::StrFormat("%.1f", process.GetCpuUsage());
    case COLUMN_TYPE:
      return process.GetIs64Bit() ? "64 bit" : "32 bit";
    default:
      return "";
  }
}

//-----------------------------------------------------------------------------
std::string ProcessesDataView::GetToolTip(int row, int /*column*/) {
  return GetProcess(row)->GetCmdLine();
}

//-----------------------------------------------------------------------------
#define ORBIT_PROC_SORT(Member)                                            \
  [&](int a, int b) {                                                      \
    return OrbitUtils::Compare(processes[a]->Member, processes[b]->Member, \
                               ascending);                                 \
  }

//-----------------------------------------------------------------------------
void ProcessesDataView::DoSort() {
  bool ascending = m_SortingOrders[m_SortingColumn] == SortingOrder::Ascending;
  std::function<bool(int a, int b)> sorter = nullptr;

  const std::vector<std::shared_ptr<Process>>& processes = process_list_;

  switch (m_SortingColumn) {
    case COLUMN_PID:
      sorter = ORBIT_PROC_SORT(GetID());
      break;
    case COLUMN_NAME:
      sorter = ORBIT_PROC_SORT(GetName());
      break;
    case COLUMN_CPU:
      sorter = ORBIT_PROC_SORT(GetCpuUsage());
      break;
    case COLUMN_TYPE:
      sorter = ORBIT_PROC_SORT(GetIs64Bit());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(m_Indices.begin(), m_Indices.end(), sorter);
  }

  SetSelectedItem();
}

//-----------------------------------------------------------------------------
void ProcessesDataView::OnSelect(int index) {
  std::shared_ptr<Process> selected_process = GetProcess(index);
  selected_process_id_ = selected_process->GetID();

  SetSelectedItem();

  if (selection_listener_) {
    selection_listener_(selected_process_id_);
  }
}

uint32_t ProcessesDataView::GetSelectedProcessId() const {
  return selected_process_id_;
}

//-----------------------------------------------------------------------------
void ProcessesDataView::SetSelectedItem() {
  for (size_t i = 0; i < GetNumElements(); ++i) {
    if (GetProcess(i)->GetID() == selected_process_id_) {
      m_SelectedIndex = i;
      return;
    }
  }

  // This happens when selected process disappears from the list.
  m_SelectedIndex = -1;
}

bool ProcessesDataView::SelectProcess(const std::string& process_name) {
  for (size_t i = 0; i < GetNumElements(); ++i) {
    std::shared_ptr<Process> process = GetProcess(i);
    // TODO: What if there are multiple processes with the same substring?
    if (process->GetFullPath().find(process_name) != std::string::npos) {
      OnSelect(i);
      Capture::GPresetToLoad = "";
      return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------
std::shared_ptr<Process> ProcessesDataView::SelectProcess(uint32_t process_id) {
  for (size_t i = 0; i < GetNumElements(); ++i) {
    std::shared_ptr<Process> process = GetProcess(i);
    if (process->GetID() == process_id) {
      OnSelect(i);
      return process;
    }
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
void ProcessesDataView::DoFilter() {
  std::vector<uint32_t> indices;
  const std::vector<std::shared_ptr<Process>>& processes = process_list_;

  std::vector<std::string> tokens = Tokenize(ToLower(m_Filter));

  for (size_t i = 0; i < processes.size(); ++i) {
    const Process& process = *processes[i];
    std::string name = ToLower(process.GetName());
    std::string type = process.GetIs64Bit() ? "64" : "32";

    bool match = true;

    for (std::string& filterToken : tokens) {
      if (!(name.find(filterToken) != std::string::npos ||
            type.find(filterToken) != std::string::npos)) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.push_back(i);
    }
  }

  m_Indices = indices;

  OnSort(m_SortingColumn, {});
}

//-----------------------------------------------------------------------------
void ProcessesDataView::UpdateProcessList() {
  size_t numProcesses = process_list_.size();
  m_Indices.resize(numProcesses);
  for (size_t i = 0; i < numProcesses; ++i) {
    m_Indices[i] = i;
  }
}

//-----------------------------------------------------------------------------
void ProcessesDataView::SetProcessList(
    const std::vector<std::shared_ptr<Process>>& process_list) {
  process_list_ = process_list;
  UpdateProcessList();
  OnSort(m_SortingColumn, {});
  OnFilter(m_Filter);
  SetSelectedItem();
}

std::shared_ptr<Process> ProcessesDataView::GetProcess(uint32_t row) const {
  return process_list_[m_Indices[row]];
}

void ProcessesDataView::SetSelectionListener(
    const std::function<void(uint32_t)>& selection_listener) {
  selection_listener_ = selection_listener;
}
