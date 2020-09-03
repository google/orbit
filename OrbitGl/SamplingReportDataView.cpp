// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingReportDataView.h"

#include <memory>
#include <set>

#include "App.h"
#include "CallStackDataView.h"
#include "FunctionUtils.h"
#include "OrbitModule.h"
#include "Path.h"
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
  const SampledFunction& func = GetSampledFunction(row);

  switch (column) {
    case kColumnSelected:
      return GOrbitApp->IsFunctionSelected(func) ? "X" : "-";
    case kColumnFunctionName:
      return func.name;
    case kColumnExclusive:
      return absl::StrFormat("%.2f", func.exclusive);
    case kColumnInclusive:
      return absl::StrFormat("%.2f", func.inclusive);
    case kColumnModuleName:
      return Path::GetFileName(func.module_path);
    case kColumnFile:
      return func.file;
    case kColumnLine:
      return func.line > 0 ? absl::StrFormat("%d", func.line) : "";
    case kColumnAddress:
      return absl::StrFormat("%#llx", func.absolute_address);
    default:
      return "";
  }
}

#define ORBIT_PROC_SORT(Member)                                                      \
  [&](int a, int b) {                                                                \
    return OrbitUtils::Compare(functions[a].Member, functions[b].Member, ascending); \
  }

#define ORBIT_CUSTOM_FUNC_SORT(Func)                                               \
  [&](int a, int b) {                                                              \
    return OrbitUtils::Compare(Func(functions[a]), Func(functions[b]), ascending); \
  }

#define ORBIT_MODULE_NAME_FUNC_SORT                                                     \
  [&](int a, int b) {                                                                   \
    return OrbitUtils::Compare(Path::GetFileName(functions[a].module_path),             \
                               Path::GetFileName(functions[b].module_path), ascending); \
  }

void SamplingReportDataView::DoSort() {
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(int a, int b)> sorter = nullptr;

  std::vector<SampledFunction>& functions = functions_;

  switch (sorting_column_) {
    case kColumnSelected:
      sorter = ORBIT_CUSTOM_FUNC_SORT(GOrbitApp->IsFunctionSelected);
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
      sorter = ORBIT_MODULE_NAME_FUNC_SORT;
      break;
    case kColumnFile:
      sorter = ORBIT_PROC_SORT(file);
      break;
    case kColumnLine:
      sorter = ORBIT_PROC_SORT(line);
      break;
    case kColumnAddress:
      sorter = ORBIT_PROC_SORT(absolute_address);
      break;
    default:
      break;
  }

  if (!sorter) {
    return;
  }

  const auto fallback_sorter = [&](const auto& ind_left, const auto& ind_right) {
    // `SampledFunction::address` is the absolute function address. Hence it is unique and qualifies
    // for total ordering.
    return functions[ind_left].absolute_address < functions[ind_right].absolute_address;
  };

  const auto combined_sorter = [&](const auto& ind_left, const auto& ind_right) {
    if (sorter(ind_left, ind_right)) {
      return true;
    }

    if (sorter(ind_right, ind_left)) {
      return false;
    }

    return fallback_sorter(ind_left, ind_right);
  };

  std::sort(indices_.begin(), indices_.end(), combined_sorter);
}

std::vector<FunctionInfo*> SamplingReportDataView::GetFunctionsFromIndices(
    const std::vector<int>& indices) {
  std::set<FunctionInfo*> functions_set;
  const std::shared_ptr<Process>& process = GOrbitApp->GetCaptureData().process();
  CHECK(process != nullptr);
  for (int index : indices) {
    SampledFunction& sampled_function = GetSampledFunction(index);
    if (sampled_function.function == nullptr) {
      sampled_function.function =
          process->GetFunctionFromAddress(sampled_function.absolute_address, false);
    }

    FunctionInfo* function = sampled_function.function;
    if (function != nullptr) {
      functions_set.insert(function);
    }
  }

  return std::vector<FunctionInfo*>(functions_set.begin(), functions_set.end());
}

std::vector<std::shared_ptr<Module>> SamplingReportDataView::GetModulesFromIndices(
    const std::vector<int>& indices) {
  std::shared_ptr<Process> process = GOrbitApp->GetCaptureData().process();
  CHECK(process != nullptr);
  std::set<std::string> module_paths;
  for (int index : indices) {
    SampledFunction& sampled_function = GetSampledFunction(index);
    module_paths.emplace(sampled_function.module_path);
  }

  std::vector<std::shared_ptr<Module>> modules;
  for (const std::string& module_path : module_paths) {
    const std::shared_ptr<Module> module = process->GetModuleFromPath(module_path);
    if (module != nullptr) {
      modules.push_back(module);
    }
  }

  return modules;
}

const std::string SamplingReportDataView::kMenuActionSelect = "Hook";
const std::string SamplingReportDataView::kMenuActionUnselect = "Unhook";
const std::string SamplingReportDataView::kMenuActionLoadSymbols = "Load Symbols";
const std::string SamplingReportDataView::kMenuActionDisassembly = "Go to Disassembly";

std::vector<std::string> SamplingReportDataView::GetContextMenu(
    int clicked_index, const std::vector<int>& selected_indices) {
  bool enable_select = false;
  bool enable_unselect = false;

  std::vector<FunctionInfo*> selected_functions = GetFunctionsFromIndices(selected_indices);

  bool enable_disassembly = !selected_functions.empty();

  for (const FunctionInfo* function : selected_functions) {
    enable_select |= !GOrbitApp->IsFunctionSelected(*function);
    enable_unselect |= GOrbitApp->IsFunctionSelected(*function);
  }

  bool enable_load = false;
  for (const auto& module : GetModulesFromIndices(selected_indices)) {
    if (!module->IsLoaded()) {
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

void SamplingReportDataView::OnContextMenu(const std::string& action, int menu_index,
                                           const std::vector<int>& item_indices) {
  if (action == kMenuActionSelect) {
    for (FunctionInfo* function : GetFunctionsFromIndices(item_indices)) {
      GOrbitApp->SelectFunction(*function);
    }
  } else if (action == kMenuActionUnselect) {
    for (FunctionInfo* function : GetFunctionsFromIndices(item_indices)) {
      GOrbitApp->DeselectFunction(*function);
    }
  } else if (action == kMenuActionLoadSymbols) {
    std::vector<std::shared_ptr<Module>> modules;
    for (const auto& module : GetModulesFromIndices(item_indices)) {
      if (!module->IsLoaded()) {
        modules.push_back(module);
      }
    }
    GOrbitApp->LoadModules(GOrbitApp->GetCaptureData().process(), modules);
  } else if (action == kMenuActionDisassembly) {
    int32_t pid = GOrbitApp->GetCaptureData().process_id();
    for (FunctionInfo* function : GetFunctionsFromIndices(item_indices)) {
      GOrbitApp->Disassemble(pid, *function);
    }
  } else {
    DataView::OnContextMenu(action, menu_index, item_indices);
  }
}

void SamplingReportDataView::OnSelect(int index) {
  SampledFunction& func = GetSampledFunction(index);
  sampling_report_->OnSelectAddress(func.absolute_address, tid_);
}

void SamplingReportDataView::LinkDataView(DataView* data_view) {
  if (data_view->GetType() == DataViewType::kCallstack) {
    callstack_data_view_ = static_cast<CallStackDataView*>(data_view);
    sampling_report_->SetCallstackDataView(callstack_data_view_);
  }
}

void SamplingReportDataView::SetSampledFunctions(const std::vector<SampledFunction>& functions) {
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
  if (tid == SamplingProfiler::kAllThreadsFakeTid) {
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
    std::string module_name = ToLower(Path::GetFileName(func.module_path));

    bool match = true;

    for (std::string& filter_token : tokens) {
      if (!(name.find(filter_token) != std::string::npos ||
            module_name.find(filter_token) != std::string::npos)) {
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

const SampledFunction& SamplingReportDataView::GetSampledFunction(unsigned int row) const {
  return functions_[indices_[row]];
}

SampledFunction& SamplingReportDataView::GetSampledFunction(unsigned int row) {
  return functions_[indices_[row]];
}
