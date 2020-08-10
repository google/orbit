// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FunctionsDataView.h"

#include "App.h"
#include "Capture.h"
#include "Core.h"
#include "FunctionUtils.h"
#include "Log.h"
#include "OrbitProcess.h"
#include "Pdb.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::FunctionInfo;

FunctionsDataView::FunctionsDataView() : DataView(DataViewType::FUNCTIONS) {}

const std::vector<DataView::Column>& FunctionsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_SELECTED] = {"Hooked", .0f, SortingOrder::Descending};
    columns[COLUMN_NAME] = {"Function", .65f, SortingOrder::Ascending};
    columns[COLUMN_SIZE] = {"Size", .0f, SortingOrder::Ascending};
    columns[COLUMN_FILE] = {"File", .0f, SortingOrder::Ascending};
    columns[COLUMN_LINE] = {"Line", .0f, SortingOrder::Ascending};
    columns[COLUMN_MODULE] = {"Module", .0f, SortingOrder::Ascending};
    columns[COLUMN_ADDRESS] = {"Address", .0f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

std::string FunctionsDataView::GetValue(int a_Row, int a_Column) {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

  if (a_Row >= static_cast<int>(GetNumElements())) {
    return "";
  }

  FunctionInfo& function = GetFunction(a_Row);

  switch (a_Column) {
    case COLUMN_SELECTED:
      return FunctionUtils::IsSelected(function) ? "X" : "-";
    case COLUMN_NAME:
      return FunctionUtils::GetDisplayName(function);
    case COLUMN_SIZE:
      return absl::StrFormat("%lu", function.size());
    case COLUMN_FILE:
      return function.file();
    case COLUMN_LINE:
      return absl::StrFormat("%i", function.line());
    case COLUMN_MODULE:
      return FunctionUtils::GetLoadedModuleName(function);
    case COLUMN_ADDRESS:
      return absl::StrFormat("0x%llx",
                             FunctionUtils::GetAbsoluteAddress(function));
    default:
      return "";
  }
}

#define ORBIT_FUNC_SORT(Member)                                            \
  [&](int a, int b) {                                                      \
    return OrbitUtils::Compare(functions[a]->Member, functions[b]->Member, \
                               ascending);                                 \
  }

#define ORBIT_CUSTOM_FUNC_SORT(Func)                                     \
  [&](int a, int b) {                                                    \
    return OrbitUtils::Compare(Func(*functions[a]), Func(*functions[b]), \
                               ascending);                               \
  }

void FunctionsDataView::DoSort() {
  // TODO(antonrohr) This sorting function can take a lot of time when a large
  // number of functions is used (several seconds). This function is currently
  // executed on the main thread and therefore freezes the UI and interrupts the
  // ssh watchdog signals that are sent to the service. Therefore this should
  // not be called on the main thread and as soon as this is done the watchdog
  // timeout should be rolled back from 25 seconds to 10 seconds in
  // OrbitService.h
  bool ascending = m_SortingOrders[m_SortingColumn] == SortingOrder::Ascending;
  std::function<bool(int a, int b)> sorter = nullptr;

  const std::vector<std::shared_ptr<FunctionInfo>>& functions =
      Capture::GTargetProcess->GetFunctions();

  switch (m_SortingColumn) {
    case COLUMN_SELECTED:
      sorter = ORBIT_CUSTOM_FUNC_SORT(FunctionUtils::IsSelected);
      break;
    case COLUMN_NAME:
      sorter = ORBIT_CUSTOM_FUNC_SORT(FunctionUtils::GetDisplayName);
      break;
    case COLUMN_SIZE:
      sorter = ORBIT_FUNC_SORT(size());
      break;
    case COLUMN_FILE:
      sorter = ORBIT_FUNC_SORT(file());
      break;
    case COLUMN_LINE:
      sorter = ORBIT_FUNC_SORT(line());
      break;
    case COLUMN_MODULE:
      sorter = ORBIT_CUSTOM_FUNC_SORT(FunctionUtils::GetLoadedModuleName);
      break;
    case COLUMN_ADDRESS:
      sorter = ORBIT_FUNC_SORT(address());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(indices_.begin(), indices_.end(), sorter);
  }
}

const std::string FunctionsDataView::MENU_ACTION_SELECT = "Hook";
const std::string FunctionsDataView::MENU_ACTION_UNSELECT = "Unhook";
const std::string FunctionsDataView::MENU_ACTION_DISASSEMBLY =
    "Go to Disassembly";

std::vector<std::string> FunctionsDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  bool enable_select = false;
  bool enable_unselect = false;
  for (int index : a_SelectedIndices) {
    const FunctionInfo& function = GetFunction(index);
    enable_select |= !FunctionUtils::IsSelected(function);
    enable_unselect |= FunctionUtils::IsSelected(function);
  }

  std::vector<std::string> menu;
  if (enable_select) menu.emplace_back(MENU_ACTION_SELECT);
  if (enable_unselect) menu.emplace_back(MENU_ACTION_UNSELECT);
  menu.emplace_back(MENU_ACTION_DISASSEMBLY);
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

void FunctionsDataView::OnContextMenu(const std::string& a_Action,
                                      int a_MenuIndex,
                                      const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_SELECT) {
    for (int i : a_ItemIndices) {
      FunctionUtils::Select(&GetFunction(i));
    }
  } else if (a_Action == MENU_ACTION_UNSELECT) {
    for (int i : a_ItemIndices) {
      FunctionUtils::UnSelect(&GetFunction(i));
    }
  } else if (a_Action == MENU_ACTION_DISASSEMBLY) {
    int32_t pid = Capture::GTargetProcess->GetID();
    for (int i : a_ItemIndices) {
      GOrbitApp->Disassemble(pid, GetFunction(i));
    }
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

void FunctionsDataView::DoFilter() {
  // TODO(antonrohr) This filter function can take a lot of time when a large
  // number of functions is used (several seconds). This function is currently
  // executed on the main thread and therefore freezes the UI and interrupts the
  // ssh watchdog signals that are sent to the service. Therefore this should
  // not be called on the main thread and as soon as this is done the watchdog
  // timeout should be rolled back from 25 seconds to 10 seconds in
  // OrbitService.h
  m_FilterTokens = absl::StrSplit(ToLower(m_Filter), ' ');

#ifdef WIN32
  ParallelFilter();
#else
  // TODO: port parallel filtering
  std::vector<uint32_t> indices;
  const std::vector<std::shared_ptr<FunctionInfo>>& functions =
      Capture::GTargetProcess->GetFunctions();
  for (size_t i = 0; i < functions.size(); ++i) {
    auto& function = functions[i];
    std::string name = ToLower(FunctionUtils::GetDisplayName(*function)) +
                       FunctionUtils::GetLoadedModuleName(*function);

    bool match = true;

    for (std::string& filterToken : m_FilterTokens) {
      if (name.find(filterToken) == std::string::npos) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.push_back(i);
    }
  }

  indices_ = indices;

  OnSort(m_SortingColumn, {});
#endif
}

void FunctionsDataView::ParallelFilter() {
#ifdef _WIN32
  const std::vector<std::shared_ptr<FunctionInfo>>& functions =
      Capture::GTargetProcess->GetFunctions();
  const auto prio = oqpi::task_priority::normal;
  auto numWorkers = oqpi_tk::scheduler().workersCount(prio);
  // int numWorkers = oqpi::thread::hardware_concurrency();
  std::vector<std::vector<int>> indicesArray;
  indicesArray.resize(numWorkers);

  oqpi_tk::parallel_for(
      "FunctionsDataViewParallelFor", functions.size(),
      [&](int32_t a_BlockIndex, int32_t a_ElementIndex) {
        std::vector<int>& result = indicesArray[a_BlockIndex];
        const std::string& name =
            ToLower(FunctionUtils::GetDisplayName(*functions[a_ElementIndex]));
        const std::string& file = functions[a_ElementIndex]->file();

        for (std::string& filterToken : m_FilterTokens) {
          if (name.find(filterToken) == std::string::npos &&
              file.find(filterToken) == std::string::npos) {
            return;
          }
        }

        result.push_back(a_ElementIndex);
      });

  std::set<int> indicesSet;
  for (std::vector<int>& results : indicesArray) {
    for (int index : results) {
      indicesSet.insert(index);
    }
  }

  indices_.clear();
  for (int i : indicesSet) {
    indices_.push_back(i);
  }
#endif
}

void FunctionsDataView::OnDataChanged() {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

  size_t numFunctions = Capture::GTargetProcess->GetFunctions().size();
  indices_.resize(numFunctions);
  for (size_t i = 0; i < numFunctions; ++i) {
    indices_[i] = i;
  }

  DataView::OnDataChanged();
}

FunctionInfo& FunctionsDataView::GetFunction(int a_Row) const {
  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());
  const std::vector<std::shared_ptr<FunctionInfo>>& functions =
      Capture::GTargetProcess->GetFunctions();
  return *functions[indices_[a_Row]];
}
