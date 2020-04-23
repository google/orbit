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
#include "Pdb.h"
#include "TcpClient.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
ProcessesDataView::ProcessesDataView() : DataView(DataViewType::PROCESSES) {
  UpdateProcessList();
  m_UpdatePeriodMs = 1000;
  m_IsRemote = false;

  GOrbitApp->RegisterProcessesDataView(this);
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
      return std::to_string((long)process.GetID());
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
std::string ProcessesDataView::GetToolTip(int a_Row, int /*a_Column*/) {
  return GetProcess(a_Row)->GetCmdLine();
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

  const std::vector<std::shared_ptr<Process>>& processes =
      m_ProcessList.GetProcesses();

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
void ProcessesDataView::OnSelect(int a_Index) {
  m_SelectedProcess = GetProcess(a_Index);
  if (!m_IsRemote) {
    m_SelectedProcess->ListModules();
  } else {
    Message msg(Msg_RemoteProcessRequest);
    msg.m_Header.m_GenericHeader.m_Address = m_SelectedProcess->GetID();
    GTcpClient->Send(msg);
  }

  UpdateModuleDataView(m_SelectedProcess);
}

void ProcessesDataView::UpdateModuleDataView(
    const std::shared_ptr<Process>& a_Process) {
  if (m_ModulesDataView) {
    m_ModulesDataView->SetProcess(a_Process);
    Capture::SetTargetProcess(a_Process);
    GOrbitApp->FireRefreshCallbacks();
  }
}

//-----------------------------------------------------------------------------
void ProcessesDataView::OnTimer() { Refresh(); }

//-----------------------------------------------------------------------------
void ProcessesDataView::Refresh() {
  if (Capture::IsCapturing()) {
    return;
  }

  if (m_RemoteProcess) {
    std::shared_ptr<Process> CurrentRemoteProcess =
        m_ProcessList.Size() == 1 ? m_ProcessList.GetProcesses()[0] : nullptr;

    if (m_RemoteProcess != CurrentRemoteProcess) {
      m_ProcessList.Clear();
      m_ProcessList.AddProcess(m_RemoteProcess);
      UpdateProcessList();
      OnFilter("");
      SelectProcess(m_RemoteProcess->GetID());
      SetSelectedItem();
    }
  } else {
    if (!m_IsRemote) {
      m_ProcessList.Refresh();
      m_ProcessList.UpdateCpuTimes();
    }
    UpdateProcessList();
    OnSort(m_SortingColumn, {});
    OnFilter(m_Filter);
    SetSelectedItem();

    if (Capture::GTargetProcess && !Capture::IsCapturing()) {
      Capture::GTargetProcess->UpdateThreadUsage();
    }
  }

  GParams.m_ProcessFilter = m_Filter;
}

//-----------------------------------------------------------------------------
void ProcessesDataView::SetSelectedItem() {
  int initialIndex = m_SelectedIndex;
  m_SelectedIndex = -1;

  for (uint32_t i = 0; i < (uint32_t)GetNumElements(); ++i) {
    if (m_SelectedProcess &&
        GetProcess(i)->GetID() == m_SelectedProcess->GetID()) {
      m_SelectedIndex = i;
      return;
    }
  }

  if (GParams.m_AutoReleasePdb && initialIndex != -1) {
    ClearSelectedProcess();
  }
}

//-----------------------------------------------------------------------------
void ProcessesDataView::ClearSelectedProcess() {
  std::shared_ptr<Process> process = std::make_shared<Process>();
  Capture::SetTargetProcess(process);
  m_ModulesDataView->SetProcess(process);
  m_SelectedProcess = process;
  GPdbDbg = nullptr;
  GOrbitApp->FireRefreshCallbacks();
}

//-----------------------------------------------------------------------------
bool ProcessesDataView::SelectProcess(const std::string& a_ProcessName) {
  for (size_t i = 0; i < GetNumElements(); ++i) {
    Process& process = *GetProcess(i);
    if (process.GetFullPath().find(a_ProcessName) != std::string::npos) {
      OnSelect(i);
      Capture::GPresetToLoad = "";
      return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------
std::shared_ptr<Process> ProcessesDataView::SelectProcess(DWORD a_ProcessId) {
  Refresh();

  for (size_t i = 0; i < GetNumElements(); ++i) {
    Process& process = *GetProcess(i);
    if (process.GetID() == a_ProcessId) {
      OnSelect(i);
      Capture::GPresetToLoad = "";
      return m_SelectedProcess;
    }
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
void ProcessesDataView::DoFilter() {
  std::vector<uint32_t> indices;
  const std::vector<std::shared_ptr<Process>>& processes =
      m_ProcessList.GetProcesses();

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
  size_t numProcesses = m_ProcessList.Size();
  m_Indices.resize(numProcesses);
  for (size_t i = 0; i < numProcesses; ++i) {
    m_Indices[i] = i;
  }
}

//-----------------------------------------------------------------------------
void ProcessesDataView::SetRemoteProcessList(ProcessList a_RemoteProcessList) {
  m_IsRemote = true;
  m_ProcessList = std::move(a_RemoteProcessList);
  UpdateProcessList();
  OnSort(m_SortingColumn, {});
  OnFilter(m_Filter);
  SetSelectedItem();
}

//-----------------------------------------------------------------------------
void ProcessesDataView::SetRemoteProcess(
    const std::shared_ptr<Process>& a_Process) {
  std::shared_ptr<Process> targetProcess =
      m_ProcessList.GetProcess(a_Process->GetID());
  if (targetProcess) {
    m_SelectedProcess = a_Process;
    UpdateModuleDataView(m_SelectedProcess);
  } else {
    m_RemoteProcess = a_Process;
  }
}

//-----------------------------------------------------------------------------
std::shared_ptr<Process> ProcessesDataView::GetProcess(
    unsigned int a_Row) const {
  return m_ProcessList.GetProcesses()[m_Indices[a_Row]];
}
