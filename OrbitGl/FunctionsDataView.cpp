// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FunctionsDataView.h"

#include "App.h"
#include "FunctionUtils.h"
#include "OrbitProcess.h"
#include "Pdb.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::FunctionInfo;

FunctionsDataView::FunctionsDataView() : DataView(DataViewType::kFunctions) {}

const std::string FunctionsDataView::kSelectedFunctionString = "✓";
const std::string FunctionsDataView::kUnselectedFunctionString = "";

const std::vector<DataView::Column>& FunctionsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnSelected] = {"Hooked", .0f, SortingOrder::kDescending};
    columns[kColumnName] = {"Function", .65f, SortingOrder::kAscending};
    columns[kColumnSize] = {"Size", .0f, SortingOrder::kAscending};
    columns[kColumnFile] = {"File", .0f, SortingOrder::kAscending};
    columns[kColumnLine] = {"Line", .0f, SortingOrder::kAscending};
    columns[kColumnModule] = {"Module", .0f, SortingOrder::kAscending};
    columns[kColumnAddress] = {"Address", .0f, SortingOrder::kAscending};
    return columns;
  }();
  return columns;
}

std::string FunctionsDataView::GetValue(int row, int column) {
  ScopeLock lock(GOrbitApp->GetSelectedProcess()->GetDataMutex());

  if (row >= static_cast<int>(GetNumElements())) {
    return "";
  }

  const FunctionInfo& function = GetFunction(row);

  switch (column) {
    case kColumnSelected:
      return GOrbitApp->IsFunctionSelected(function) ? kSelectedFunctionString
                                                     : kUnselectedFunctionString;
    case kColumnName:
      return FunctionUtils::GetDisplayName(function);
    case kColumnSize:
      return absl::StrFormat("%lu", function.size());
    case kColumnFile:
      return function.file();
    case kColumnLine:
      return absl::StrFormat("%i", function.line());
    case kColumnModule:
      return FunctionUtils::GetLoadedModuleName(function);
    case kColumnAddress:
      return absl::StrFormat("0x%llx", FunctionUtils::GetAbsoluteAddress(function));
    default:
      return "";
  }
}

#define ORBIT_FUNC_SORT(Member)                                                        \
  [&](int a, int b) {                                                                  \
    return OrbitUtils::Compare(functions[a]->Member, functions[b]->Member, ascending); \
  }

#define ORBIT_CUSTOM_FUNC_SORT(Func)                                                 \
  [&](int a, int b) {                                                                \
    return OrbitUtils::Compare(Func(*functions[a]), Func(*functions[b]), ascending); \
  }

void FunctionsDataView::DoSort() {
  // TODO(antonrohr) This sorting function can take a lot of time when a large
  // number of functions is used (several seconds). This function is currently
  // executed on the main thread and therefore freezes the UI and interrupts the
  // ssh watchdog signals that are sent to the service. Therefore this should
  // not be called on the main thread and as soon as this is done the watchdog
  // timeout should be rolled back from 25 seconds to 10 seconds in
  // OrbitService.h
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(int a, int b)> sorter = nullptr;

  const std::vector<std::shared_ptr<FunctionInfo>>& functions =
      GOrbitApp->GetSelectedProcess()->GetFunctions();

  switch (sorting_column_) {
    case kColumnSelected:
      sorter = ORBIT_CUSTOM_FUNC_SORT(GOrbitApp->IsFunctionSelected);
      break;
    case kColumnName:
      sorter = ORBIT_CUSTOM_FUNC_SORT(FunctionUtils::GetDisplayName);
      break;
    case kColumnSize:
      sorter = ORBIT_FUNC_SORT(size());
      break;
    case kColumnFile:
      sorter = ORBIT_FUNC_SORT(file());
      break;
    case kColumnLine:
      sorter = ORBIT_FUNC_SORT(line());
      break;
    case kColumnModule:
      sorter = ORBIT_CUSTOM_FUNC_SORT(FunctionUtils::GetLoadedModuleName);
      break;
    case kColumnAddress:
      sorter = ORBIT_FUNC_SORT(address());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(indices_.begin(), indices_.end(), sorter);
  }
}

const std::string FunctionsDataView::kMenuActionSelect = "Hook";
const std::string FunctionsDataView::kMenuActionUnselect = "Unhook";
const std::string FunctionsDataView::kMenuActionDisassembly = "Go to Disassembly";

std::vector<std::string> FunctionsDataView::GetContextMenu(
    int clicked_index, const std::vector<int>& selected_indices) {
  bool enable_select = false;
  bool enable_unselect = false;
  for (int index : selected_indices) {
    const FunctionInfo& function = GetFunction(index);
    enable_select |= !GOrbitApp->IsFunctionSelected(function);
    enable_unselect |= GOrbitApp->IsFunctionSelected(function);
  }

  std::vector<std::string> menu;
  if (enable_select) menu.emplace_back(kMenuActionSelect);
  if (enable_unselect) menu.emplace_back(kMenuActionUnselect);
  menu.emplace_back(kMenuActionDisassembly);
  Append(menu, DataView::GetContextMenu(clicked_index, selected_indices));
  return menu;
}

void FunctionsDataView::OnContextMenu(const std::string& action, int menu_index,
                                      const std::vector<int>& item_indices) {
  if (action == kMenuActionSelect) {
    for (int i : item_indices) {
      GOrbitApp->SelectFunction(GetFunction(i));
    }
  } else if (action == kMenuActionUnselect) {
    for (int i : item_indices) {
      GOrbitApp->DeselectFunction(GetFunction(i));
    }
  } else if (action == kMenuActionDisassembly) {
    int32_t pid = GOrbitApp->GetSelectedProcessId();
    for (int i : item_indices) {
      GOrbitApp->Disassemble(pid, GetFunction(i));
    }
  } else {
    DataView::OnContextMenu(action, menu_index, item_indices);
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
  m_FilterTokens = absl::StrSplit(ToLower(filter_), ' ');

#ifdef WIN32
  ParallelFilter();
#else
  // TODO: port parallel filtering
  std::vector<uint32_t> indices;
  const std::vector<std::shared_ptr<FunctionInfo>>& functions =
      GOrbitApp->GetSelectedProcess()->GetFunctions();
  for (size_t i = 0; i < functions.size(); ++i) {
    auto& function = functions[i];
    std::string name = ToLower(FunctionUtils::GetDisplayName(*function)) +
                       FunctionUtils::GetLoadedModuleName(*function);

    bool match = true;

    for (std::string& filter_token : m_FilterTokens) {
      if (name.find(filter_token) == std::string::npos) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.push_back(i);
    }
  }

  indices_ = indices;

  OnSort(sorting_column_, {});
#endif
}

void FunctionsDataView::ParallelFilter() {
#ifdef _WIN32
  const std::vector<std::shared_ptr<FunctionInfo>>& functions =
      GOrbitApp->GetSelectedProcess()->GetFunctions();
  const auto prio = oqpi::task_priority::normal;
  auto numWorkers = oqpi_tk::scheduler().workersCount(prio);
  // int numWorkers = oqpi::thread::hardware_concurrency();
  std::vector<std::vector<int>> indicesArray;
  indicesArray.resize(numWorkers);

  oqpi_tk::parallel_for("FunctionsDataViewParallelFor", functions.size(),
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
  ScopeLock lock(GOrbitApp->GetSelectedProcess()->GetDataMutex());

  size_t num_functions = GOrbitApp->GetSelectedProcess()->GetFunctions().size();
  indices_.resize(num_functions);
  for (size_t i = 0; i < num_functions; ++i) {
    indices_[i] = i;
  }

  DataView::OnDataChanged();
}

FunctionInfo& FunctionsDataView::GetFunction(int row) const {
  ScopeLock lock(GOrbitApp->GetSelectedProcess()->GetDataMutex());
  const std::vector<std::shared_ptr<FunctionInfo>>& functions =
      GOrbitApp->GetSelectedProcess()->GetFunctions();
  return *functions[indices_[row]];
}
