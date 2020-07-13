// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingReportDataView.h"

#include <memory>
#include <set>

#include "App.h"
#include "CallStackDataView.h"
#include "Capture.h"
#include "Core.h"
#include "FunctionUtils.h"
#include "OrbitModule.h"
#include "SamplingReport.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
SamplingReportDataView::SamplingReportDataView()
    : DataView(DataViewType::SAMPLING), m_CallstackDataView(nullptr) {}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& SamplingReportDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_SELECTED] = {"Hooked", .0f, SortingOrder::Descending};
    columns[COLUMN_FUNCTION_NAME] = {"Name", .5f, SortingOrder::Ascending};
    columns[COLUMN_EXCLUSIVE] = {"Exclusive", .0f, SortingOrder::Descending};
    columns[COLUMN_INCLUSIVE] = {"Inclusive", .0f, SortingOrder::Descending};
    columns[COLUMN_MODULE_NAME] = {"Module", .0f, SortingOrder::Ascending};
    columns[COLUMN_FILE] = {"File", .0f, SortingOrder::Ascending};
    columns[COLUMN_LINE] = {"Line", .0f, SortingOrder::Ascending};
    columns[COLUMN_ADDRESS] = {"Address", .0f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string SamplingReportDataView::GetValue(int a_Row, int a_Column) {
  SampledFunction& func = GetSampledFunction(a_Row);

  switch (a_Column) {
    case COLUMN_SELECTED:
      return FunctionUtils::IsSelected(func) ? "X" : "-";
    case COLUMN_FUNCTION_NAME:
      return func.name();
    case COLUMN_EXCLUSIVE:
      return absl::StrFormat("%.2f", func.exclusive());
    case COLUMN_INCLUSIVE:
      return absl::StrFormat("%.2f", func.inclusive());
    case COLUMN_MODULE_NAME:
      return func.module();
    case COLUMN_FILE:
      return func.file();
    case COLUMN_LINE:
      return func.line() > 0 ? absl::StrFormat("%d", func.line()) : "";
    case COLUMN_ADDRESS:
      return absl::StrFormat("%#llx", func.address());
    default:
      return "";
  }
}

//-----------------------------------------------------------------------------
#define ORBIT_PROC_SORT(Member)                                          \
  [&](int a, int b) {                                                    \
    return OrbitUtils::Compare(functions[a].Member, functions[b].Member, \
                               ascending);                               \
  }

//-----------------------------------------------------------------------------
#define ORBIT_CUSTOM_FUNC_SORT(Func)                                   \
  [&](int a, int b) {                                                  \
    return OrbitUtils::Compare(Func(functions[a]), Func(functions[b]), \
                               ascending);                             \
  }

//-----------------------------------------------------------------------------
void SamplingReportDataView::DoSort() {
  bool ascending = m_SortingOrders[m_SortingColumn] == SortingOrder::Ascending;
  std::function<bool(int a, int b)> sorter = nullptr;

  std::vector<SampledFunction>& functions = m_Functions;

  switch (m_SortingColumn) {
    case COLUMN_SELECTED:
      sorter = ORBIT_CUSTOM_FUNC_SORT(FunctionUtils::IsSelected);
      break;
    case COLUMN_FUNCTION_NAME:
      sorter = ORBIT_PROC_SORT(name());
      break;
    case COLUMN_EXCLUSIVE:
      sorter = ORBIT_PROC_SORT(exclusive());
      break;
    case COLUMN_INCLUSIVE:
      sorter = ORBIT_PROC_SORT(inclusive());
      break;
    case COLUMN_MODULE_NAME:
      sorter = ORBIT_PROC_SORT(module());
      break;
    case COLUMN_FILE:
      sorter = ORBIT_PROC_SORT(file());
      break;
    case COLUMN_LINE:
      sorter = ORBIT_PROC_SORT(line());
      break;
    case COLUMN_ADDRESS:
      sorter = ORBIT_PROC_SORT(address());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(m_Indices.begin(), m_Indices.end(), sorter);
  }
}

//-----------------------------------------------------------------------------
std::vector<Function*> SamplingReportDataView::GetFunctionsFromIndices(
    const std::vector<int>& a_Indices) {
  std::set<Function*> functions_set;
  if (Capture::GTargetProcess != nullptr) {
    for (int index : a_Indices) {
      SampledFunction& sampled_function = GetSampledFunction(index);
      if (!sampled_function.has_function()) {
          Function* function = Capture::GTargetProcess->GetFunctionFromAddress(
                sampled_function.address(), false);
          if (function != nullptr) {
            (*sampled_function.mutable_function()).CopyFrom(*function);
          }
      }

      if (sampled_function.has_function()) {
        functions_set.insert(sampled_function.mutable_function());
      }
    }
  }

  return std::vector<Function*>(functions_set.begin(), functions_set.end());
}

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<Module>>
SamplingReportDataView::GetModulesFromIndices(
    const std::vector<int>& a_Indices) {
  std::vector<std::shared_ptr<Module>> modules;
  if (Capture::GTargetProcess != nullptr) {
    std::set<std::string> module_names;
    for (int index : a_Indices) {
      SampledFunction& sampled_function = GetSampledFunction(index);
      module_names.emplace(sampled_function.module());
    }

    auto& module_map = Capture::GTargetProcess->GetNameToModulesMap();
    for (const std::string& module_name : module_names) {
      auto module_it = module_map.find(ToLower(module_name));
      if (module_it != module_map.end()) {
        modules.push_back(module_it->second);
      }
    }
  }
  return modules;
}

//-----------------------------------------------------------------------------
const std::string SamplingReportDataView::MENU_ACTION_SELECT = "Hook";
const std::string SamplingReportDataView::MENU_ACTION_UNSELECT = "Unhook";
const std::string SamplingReportDataView::MENU_ACTION_MODULES_LOAD =
    "Load Symbols";
const std::string SamplingReportDataView::MENU_ACTION_DISASSEMBLY =
    "Go to Disassembly";

//-----------------------------------------------------------------------------
std::vector<std::string> SamplingReportDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  bool enable_select = false;
  bool enable_unselect = false;

  std::vector<Function*> selected_functions =
      GetFunctionsFromIndices(a_SelectedIndices);

  bool enable_disassembly = !selected_functions.empty();

  for (const Function* function : selected_functions) {
    enable_select |= !FunctionUtils::IsSelected(*function);
    enable_unselect |= FunctionUtils::IsSelected(*function);
  }

  bool enable_load = false;
  for (const auto& module : GetModulesFromIndices(a_SelectedIndices)) {
    if (module->IsLoadable() && !module->IsLoaded()) {
      enable_load = true;
    }
  }

  std::vector<std::string> menu;
  if (enable_select) menu.emplace_back(MENU_ACTION_SELECT);
  if (enable_unselect) menu.emplace_back(MENU_ACTION_UNSELECT);
  if (enable_load) menu.emplace_back(MENU_ACTION_MODULES_LOAD);
  if (enable_disassembly) menu.emplace_back(MENU_ACTION_DISASSEMBLY);
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::OnContextMenu(
    const std::string& a_Action, int a_MenuIndex,
    const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_SELECT) {
    for (Function* function : GetFunctionsFromIndices(a_ItemIndices)) {
      FunctionUtils::Select(function);
    }
  } else if (a_Action == MENU_ACTION_UNSELECT) {
    for (Function* function : GetFunctionsFromIndices(a_ItemIndices)) {
      FunctionUtils::UnSelect(function);
    }
  } else if (a_Action == MENU_ACTION_MODULES_LOAD) {
    std::vector<std::shared_ptr<Module>> modules;
    for (const auto& module : GetModulesFromIndices(a_ItemIndices)) {
      if (module->IsLoadable() && !module->IsLoaded()) {
        modules.push_back(module);
      }
    }
    GOrbitApp->LoadModules(Capture::GTargetProcess->GetID(), modules);
  } else if (a_Action == MENU_ACTION_DISASSEMBLY) {
    int32_t pid = Capture::GTargetProcess->GetID();
    for (Function* function : GetFunctionsFromIndices(a_ItemIndices)) {
      GOrbitApp->Disassemble(pid, *function);
    }
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::OnSelect(int index) {
  SampledFunction& func = GetSampledFunction(index);
  m_SamplingReport->OnSelectAddress(func.address(), m_TID);
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::LinkDataView(DataView* a_DataView) {
  if (a_DataView->GetType() == DataViewType::CALLSTACK) {
    m_CallstackDataView = static_cast<CallStackDataView*>(a_DataView);
    m_SamplingReport->SetCallstackDataView(m_CallstackDataView);
  }
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::SetSampledFunctions(
    const google::protobuf::RepeatedPtrField<SampledFunction>& a_Functions) {
  m_Functions = {a_Functions.begin(), a_Functions.end()};

  size_t numFunctions = m_Functions.size();
  m_Indices.resize(numFunctions);
  for (size_t i = 0; i < numFunctions; ++i) {
    m_Indices[i] = i;
  }

  OnDataChanged();
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::SetThreadID(ThreadID a_TID) {
  m_TID = a_TID;
  if (a_TID == 0) {
    m_Name = "All";
  } else {
    m_Name = absl::StrFormat("%d", m_TID);
  }
}

//-----------------------------------------------------------------------------
void SamplingReportDataView::DoFilter() {
  std::vector<uint32_t> indices;

  std::vector<std::string> tokens = absl::StrSplit(ToLower(m_Filter), ' ');

  for (size_t i = 0; i < m_Functions.size(); ++i) {
    SampledFunction& func = m_Functions[i];
    std::string name = ToLower(func.name());
    std::string module = ToLower(func.module());

    bool match = true;

    for (std::string& filterToken : tokens) {
      if (!(name.find(filterToken) != std::string::npos ||
            module.find(filterToken) != std::string::npos)) {
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
const SampledFunction& SamplingReportDataView::GetSampledFunction(
    unsigned int a_Row) const {
  return m_Functions[m_Indices[a_Row]];
}

//-----------------------------------------------------------------------------
SampledFunction& SamplingReportDataView::GetSampledFunction(
    unsigned int a_Row) {
  return m_Functions[m_Indices[a_Row]];
}
