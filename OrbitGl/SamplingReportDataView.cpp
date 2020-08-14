// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingReportDataView.h"

#include <memory>
#include <set>

#include "App.h"
#include "CallStackDataView.h"
#include "Capture.h"
#include "FunctionUtils.h"
#include "OrbitModule.h"
#include "SamplingReport.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::FunctionInfo;

SamplingReportDataView::SamplingReportDataView()
    : DataView(DataViewType::kSampling), callstack_data_view_(nullptr) {}

const std::vector<DataView::Column>& SamplingReportDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnSelected] = {"Hooked", .0f, SortingOrder::kDescending};
    columns[kColumnFunctionName] = {"Name", .5f, SortingOrder::kAscending};
    columns[kColumnExclusive] = {"Exclusive", .0f, SortingOrder::kDescending};
    columns[kColumnInclusive] = {"Inclusive", .0f, SortingOrder::kDescending};
    columns[kColumnModuleName] = {"Module", .0f, SortingOrder::kAscending};
    columns[kColumnFile] = {"File", .0f, SortingOrder::kAscending};
    columns[kColumnLine] = {"Line", .0f, SortingOrder::kAscending};
    columns[kColumnAddress] = {"Address", .0f, SortingOrder::kAscending};
    return columns;
  }();
  return columns;
}

std::string SamplingReportDataView::GetValue(int row, int column) {
  SampledFunction& func = GetSampledFunction(row);

  switch (column) {
    case kColumnSelected:
      return FunctionUtils::IsSelected(func) ? "X" : "-";
    case kColumnFunctionName:
      return func.name;
    case kColumnExclusive:
      return absl::StrFormat("%.2f", func.exclusive);
    case kColumnInclusive:
      return absl::StrFormat("%.2f", func.inclusive);
    case kColumnModuleName:
      return func.module;
    case kColumnFile:
      return func.file;
    case kColumnLine:
      return func.line > 0 ? absl::StrFormat("%d", func.line) : "";
    case kColumnAddress:
      return absl::StrFormat("%#llx", func.address);
    default:
      return "";
  }
}

#define ORBIT_PROC_SORT(Member)                                          \
  [&](int a, int b) {                                                    \
    return OrbitUtils::Compare(functions[a].Member, functions[b].Member, \
                               ascending);                               \
  }

#define ORBIT_CUSTOM_FUNC_SORT(Func)                                   \
  [&](int a, int b) {                                                  \
    return OrbitUtils::Compare(Func(functions[a]), Func(functions[b]), \
                               ascending);                             \
  }

void SamplingReportDataView::DoSort() {
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(int a, int b)> sorter = nullptr;

  std::vector<SampledFunction>& functions = functions_;

  switch (sorting_column_) {
    case kColumnSelected:
      sorter = ORBIT_CUSTOM_FUNC_SORT(FunctionUtils::IsSelected);
      break;
    case kColumnFunctionName:
      sorter = ORBIT_PROC_SORT(name);
      break;
    case kColumnExclusive:
      sorter = ORBIT_PROC_SORT(exclusive);
      break;
    case kColumnInclusive:
      sorter = ORBIT_PROC_SORT(inclusive);
      break;
    case kColumnModuleName:
      sorter = ORBIT_PROC_SORT(module);
      break;
    case kColumnFile:
      sorter = ORBIT_PROC_SORT(file);
      break;
    case kColumnLine:
      sorter = ORBIT_PROC_SORT(line);
      break;
    case kColumnAddress:
      sorter = ORBIT_PROC_SORT(address);
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(indices_.begin(), indices_.end(), sorter);
  }
}

std::vector<FunctionInfo*> SamplingReportDataView::GetFunctionsFromIndices(
    const std::vector<int>& indices) {
  std::set<FunctionInfo*> functions_set;
  if (Capture::GTargetProcess != nullptr) {
    for (int index : indices) {
      SampledFunction& sampled_function = GetSampledFunction(index);
      if (sampled_function.function == nullptr) {
        sampled_function.function =
            Capture::GTargetProcess->GetFunctionFromAddress(
                sampled_function.address, false);
      }

      FunctionInfo* function = sampled_function.function;
      if (function != nullptr) {
        functions_set.insert(function);
      }
    }
  }

  return std::vector<FunctionInfo*>(functions_set.begin(), functions_set.end());
}

std::vector<std::shared_ptr<Module>>
SamplingReportDataView::GetModulesFromIndices(const std::vector<int>& indices) {
  std::vector<std::shared_ptr<Module>> modules;
  if (Capture::GTargetProcess != nullptr) {
    std::set<std::string> module_names;
    for (int index : indices) {
      SampledFunction& sampled_function = GetSampledFunction(index);
      module_names.emplace(sampled_function.module);
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

const std::string SamplingReportDataView::kMenuActionSelect = "Hook";
const std::string SamplingReportDataView::kMenuActionUnselect = "Unhook";
const std::string SamplingReportDataView::kMenuActionLoadSymbols =
    "Load Symbols";
const std::string SamplingReportDataView::kMenuActionDisassembly =
    "Go to Disassembly";

std::vector<std::string> SamplingReportDataView::GetContextMenu(
    int clicked_index, const std::vector<int>& selected_indices) {
  bool enable_select = false;
  bool enable_unselect = false;

  std::vector<FunctionInfo*> selected_functions =
      GetFunctionsFromIndices(selected_indices);

  bool enable_disassembly = !selected_functions.empty();

  for (const FunctionInfo* function : selected_functions) {
    enable_select |= !FunctionUtils::IsSelected(*function);
    enable_unselect |= FunctionUtils::IsSelected(*function);
  }

  bool enable_load = false;
  for (const auto& module : GetModulesFromIndices(selected_indices)) {
    if (module->IsLoadable() && !module->IsLoaded()) {
      enable_load = true;
    }
  }

  std::vector<std::string> menu;
  if (enable_select) menu.emplace_back(kMenuActionSelect);
  if (enable_unselect) menu.emplace_back(kMenuActionUnselect);
  if (enable_load) menu.emplace_back(kMenuActionLoadSymbols);
  if (enable_disassembly) menu.emplace_back(kMenuActionDisassembly);
  Append(menu, DataView::GetContextMenu(clicked_index, selected_indices));
  return menu;
}

void SamplingReportDataView::OnContextMenu(
    const std::string& action, int menu_index,
    const std::vector<int>& item_indices) {
  if (action == kMenuActionSelect) {
    for (FunctionInfo* function : GetFunctionsFromIndices(item_indices)) {
      FunctionUtils::Select(function);
    }
  } else if (action == kMenuActionUnselect) {
    for (FunctionInfo* function : GetFunctionsFromIndices(item_indices)) {
      FunctionUtils::UnSelect(function);
    }
  } else if (action == kMenuActionLoadSymbols) {
    std::vector<std::shared_ptr<Module>> modules;
    for (const auto& module : GetModulesFromIndices(item_indices)) {
      if (module->IsLoadable() && !module->IsLoaded()) {
        modules.push_back(module);
      }
    }
    GOrbitApp->LoadModules(Capture::GTargetProcess->GetID(), modules);
  } else if (action == kMenuActionDisassembly) {
    int32_t pid = Capture::GTargetProcess->GetID();
    for (FunctionInfo* function : GetFunctionsFromIndices(item_indices)) {
      GOrbitApp->Disassemble(pid, *function);
    }
  } else {
    DataView::OnContextMenu(action, menu_index, item_indices);
  }
}

void SamplingReportDataView::OnSelect(int index) {
  SampledFunction& func = GetSampledFunction(index);
  sampling_report_->OnSelectAddress(func.address, tid_);
}

void SamplingReportDataView::LinkDataView(DataView* data_view) {
  if (data_view->GetType() == DataViewType::kCallstack) {
    callstack_data_view_ = static_cast<CallStackDataView*>(data_view);
    sampling_report_->SetCallstackDataView(callstack_data_view_);
  }
}

void SamplingReportDataView::SetSampledFunctions(
    const std::vector<SampledFunction>& functions) {
  functions_ = functions;

  size_t num_functions = functions_.size();
  indices_.resize(num_functions);
  for (size_t i = 0; i < num_functions; ++i) {
    indices_[i] = i;
  }

  OnDataChanged();
}

void SamplingReportDataView::SetThreadID(ThreadID tid) {
  tid_ = tid;
  if (tid == 0) {
    name_ = "All";
  } else {
    name_ = absl::StrFormat("%d", tid_);
  }
}

void SamplingReportDataView::DoFilter() {
  std::vector<uint32_t> indices;

  std::vector<std::string> tokens = absl::StrSplit(ToLower(filter_), ' ');

  for (size_t i = 0; i < functions_.size(); ++i) {
    SampledFunction& func = functions_[i];
    std::string name = ToLower(func.name);
    std::string module = ToLower(func.module);

    bool match = true;

    for (std::string& filter_token : tokens) {
      if (!(name.find(filter_token) != std::string::npos ||
            module.find(filter_token) != std::string::npos)) {
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
}

const SampledFunction& SamplingReportDataView::GetSampledFunction(
    unsigned int row) const {
  return functions_[indices_[row]];
}

SampledFunction& SamplingReportDataView::GetSampledFunction(unsigned int row) {
  return functions_[indices_[row]];
}
