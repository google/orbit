// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingReportDataView.h"

#include <absl/container/flat_hash_set.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_split.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <outcome.hpp>
#include <utility>

#include "App.h"
#include "CallstackDataView.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "ClientModel/CaptureData.h"
#include "CoreUtils.h"
#include "DataViewTypes.h"
#include "FunctionsDataView.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadConstants.h"
#include "SamplingReport.h"
#include "absl/strings/str_format.h"

using orbit_client_data::ModuleData;
using orbit_client_data::ProcessData;
using orbit_client_data::SampledFunction;
using orbit_client_data::ThreadID;
using orbit_client_model::CaptureData;
using orbit_client_protos::FunctionInfo;

ABSL_DECLARE_FLAG(bool, enable_source_code_view);

SamplingReportDataView::SamplingReportDataView(OrbitApp* app)
    : DataView(DataViewType::kSampling, app), callstack_data_view_(nullptr) {}

const std::vector<DataView::Column>& SamplingReportDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(kNumColumns);
    columns[kColumnSelected] = {"Hooked", .0f, SortingOrder::kDescending};
    columns[kColumnFunctionName] = {"Name", .5f, SortingOrder::kAscending};
    columns[kColumnExclusive] = {"Exclusive", .0f, SortingOrder::kDescending};
    columns[kColumnInclusive] = {"Inclusive", .0f, SortingOrder::kDescending};
    columns[kColumnModuleName] = {"Module", .0f, SortingOrder::kAscending};
    columns[kColumnAddress] = {"Address", .0f, SortingOrder::kAscending};
    return columns;
  }();
  return columns;
}

std::string SamplingReportDataView::GetValue(int row, int column) {
  const SampledFunction& func = GetSampledFunction(row);

  switch (column) {
    case kColumnSelected:
      return app_->IsFunctionSelected(func) ? FunctionsDataView::kSelectedFunctionString
                                            : FunctionsDataView::kUnselectedFunctionString;
    case kColumnFunctionName:
      return func.name;
    case kColumnExclusive:
      return absl::StrFormat("%.2f%% (%u)", func.exclusive_percent, func.exclusive);
    case kColumnInclusive:
      return absl::StrFormat("%.2f%% (%u)", func.inclusive_percent, func.inclusive);
    case kColumnModuleName:
      return std::filesystem::path(func.module_path).filename().string();
    case kColumnAddress:
      return absl::StrFormat("%#llx", func.absolute_address);
    default:
      return "";
  }
}

#define ORBIT_PROC_SORT(Member)                                                      \
  [&](int a, int b) {                                                                \
    return orbit_core::Compare(functions[a].Member, functions[b].Member, ascending); \
  }

#define ORBIT_CUSTOM_FUNC_SORT(Func)                                               \
  [&](int a, int b) {                                                              \
    return orbit_core::Compare(Func(functions[a]), Func(functions[b]), ascending); \
  }

#define ORBIT_MODULE_NAME_FUNC_SORT                                                        \
  [&](int a, int b) {                                                                      \
    return orbit_core::Compare(std::filesystem::path(functions[a].module_path).filename(), \
                               std::filesystem::path(functions[b].module_path).filename(), \
                               ascending);                                                 \
  }

void SamplingReportDataView::DoSort() {
  bool ascending = sorting_orders_[sorting_column_] == SortingOrder::kAscending;
  std::function<bool(int a, int b)> sorter = nullptr;

  std::vector<SampledFunction>& functions = functions_;

  switch (sorting_column_) {
    case kColumnSelected:
      sorter = ORBIT_CUSTOM_FUNC_SORT(app_->IsFunctionSelected);
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

absl::flat_hash_set<const FunctionInfo*> SamplingReportDataView::GetFunctionsFromIndices(
    const std::vector<int>& indices) {
  absl::flat_hash_set<const FunctionInfo*> functions_set;
  const CaptureData& capture_data = app_->GetCaptureData();
  for (int index : indices) {
    SampledFunction& sampled_function = GetSampledFunction(index);
    if (sampled_function.function == nullptr) {
      const FunctionInfo* func =
          capture_data.FindFunctionByAddress(sampled_function.absolute_address, false);
      sampled_function.function = func;
    }

    const FunctionInfo* function = sampled_function.function;
    if (function != nullptr) {
      functions_set.insert(function);
    }
  }

  return functions_set;
}

absl::flat_hash_set<std::pair<std::string, std::string>>
SamplingReportDataView::GetModulePathsAndBuildIdsFromIndices(
    const std::vector<int>& indices) const {
  absl::flat_hash_set<std::pair<std::string, std::string>> module_paths_and_build_ids;
  const ProcessData* process = app_->GetCaptureData().process();
  CHECK(process != nullptr);

  for (int index : indices) {
    const SampledFunction& sampled_function = GetSampledFunction(index);
    CHECK(sampled_function.absolute_address != 0);
    auto result = process->FindModuleByAddress(sampled_function.absolute_address);
    if (result.has_error()) {
      ERROR("result %s", result.error().message());
    } else {
      module_paths_and_build_ids.emplace(result.value().file_path(), result.value().build_id());
    }
  }

  return module_paths_and_build_ids;
}

const std::string SamplingReportDataView::kMenuActionSelect = "Hook";
const std::string SamplingReportDataView::kMenuActionUnselect = "Unhook";
const std::string SamplingReportDataView::kMenuActionLoadSymbols = "Load Symbols";
const std::string SamplingReportDataView::kMenuActionDisassembly = "Go to Disassembly";
const std::string SamplingReportDataView::kMenuActionSourceCode = "Go to Source code";

std::vector<std::string> SamplingReportDataView::GetContextMenu(
    int clicked_index, const std::vector<int>& selected_indices) {
  bool enable_select = false;
  bool enable_unselect = false;
  bool enable_disassembly = false;
  bool enable_source_code = false;

  if (app_->IsCaptureConnected(app_->GetCaptureData())) {
    absl::flat_hash_set<const FunctionInfo*> selected_functions =
        GetFunctionsFromIndices(selected_indices);

    enable_disassembly = !selected_functions.empty();
    enable_source_code =
        !selected_functions.empty() && absl::GetFlag(FLAGS_enable_source_code_view);

    for (const FunctionInfo* function : selected_functions) {
      enable_select |= !app_->IsFunctionSelected(*function);
      enable_unselect |= app_->IsFunctionSelected(*function);
    }
  }

  bool enable_load = false;
  for (const auto& [module_path, build_id] :
       GetModulePathsAndBuildIdsFromIndices(selected_indices)) {
    const ModuleData* module = app_->GetModuleByPathAndBuildId(module_path, build_id);
    if (!module->is_loaded()) {
      enable_load = true;
    }
  }

  std::vector<std::string> menu;
  if (enable_select) menu.emplace_back(kMenuActionSelect);
  if (enable_unselect) menu.emplace_back(kMenuActionUnselect);
  if (enable_load) menu.emplace_back(kMenuActionLoadSymbols);
  if (enable_disassembly) menu.emplace_back(kMenuActionDisassembly);
  if (enable_source_code) menu.emplace_back(kMenuActionSourceCode);
  Append(menu, DataView::GetContextMenu(clicked_index, selected_indices));
  return menu;
}

void SamplingReportDataView::OnContextMenu(const std::string& action, int menu_index,
                                           const std::vector<int>& item_indices) {
  if (action == kMenuActionSelect) {
    for (const FunctionInfo* function : GetFunctionsFromIndices(item_indices)) {
      app_->SelectFunction(*function);
    }
  } else if (action == kMenuActionUnselect) {
    for (const FunctionInfo* function : GetFunctionsFromIndices(item_indices)) {
      app_->DeselectFunction(*function);
      app_->DisableFrameTrack(*function);
    }
  } else if (action == kMenuActionLoadSymbols) {
    std::vector<ModuleData*> modules_to_load;
    for (const auto& [module_path, build_id] : GetModulePathsAndBuildIdsFromIndices(item_indices)) {
      ModuleData* module = app_->GetMutableModuleByPathAndBuildId(module_path, build_id);
      if (!module->is_loaded()) {
        modules_to_load.push_back(module);
      }
    }
    app_->RetrieveModulesAndLoadSymbols(modules_to_load);
  } else if (action == kMenuActionDisassembly) {
    int32_t pid = app_->GetCaptureData().process_id();
    for (const FunctionInfo* function : GetFunctionsFromIndices(item_indices)) {
      app_->Disassemble(pid, *function);
    }
  } else if (action == kMenuActionSourceCode) {
    for (const FunctionInfo* function : GetFunctionsFromIndices(item_indices)) {
      app_->ShowSourceCode(*function);
    }
  } else {
    DataView::OnContextMenu(action, menu_index, item_indices);
  }
}

void SamplingReportDataView::UpdateSelectedIndicesAndFunctionIds(
    const std::vector<int>& selected_indices) {
  selected_indices_.clear();
  selected_function_ids_.clear();
  for (int row : selected_indices) {
    selected_indices_.insert(indices_[row]);
    selected_function_ids_.insert(GetSampledFunction(row).absolute_address);
  }
}

void SamplingReportDataView::RestoreSelectedIndicesAfterFunctionsChanged() {
  selected_indices_.clear();
  for (size_t row = 0; row < functions_.size(); ++row) {
    if (selected_function_ids_.contains(functions_[row].absolute_address)) {
      selected_indices_.insert(static_cast<int>(row));
    }
  }
}

void SamplingReportDataView::UpdateVisibleSelectedAddressesAndTid(
    const std::vector<int>& visible_selected_indices) {
  absl::flat_hash_set<uint64_t> addresses;
  for (int index : visible_selected_indices) {
    addresses.insert(GetSampledFunction(index).absolute_address);
  }
  sampling_report_->OnSelectAddresses(addresses, tid_);
}

void SamplingReportDataView::OnSelect(const std::vector<int>& indices) {
  UpdateSelectedIndicesAndFunctionIds(indices);
  UpdateVisibleSelectedAddressesAndTid(indices);
}

void SamplingReportDataView::OnRefresh(const std::vector<int>& visible_selected_indices,
                                       const RefreshMode& mode) {
  if (mode != RefreshMode::kOnFilter && mode != RefreshMode::kOnSort) return;
  UpdateVisibleSelectedAddressesAndTid(visible_selected_indices);
}

void SamplingReportDataView::LinkDataView(DataView* data_view) {
  if (data_view->GetType() == DataViewType::kCallstack) {
    callstack_data_view_ = static_cast<CallstackDataView*>(data_view);
    sampling_report_->SetCallstackDataView(callstack_data_view_);
  }
}

void SamplingReportDataView::SetSampledFunctions(const std::vector<SampledFunction>& functions) {
  functions_ = functions;
  RestoreSelectedIndicesAfterFunctionsChanged();

  size_t num_functions = functions_.size();
  indices_.resize(num_functions);
  for (size_t i = 0; i < num_functions; ++i) {
    indices_[i] = i;
  }

  OnDataChanged();
}

void SamplingReportDataView::SetThreadID(ThreadID tid) {
  tid_ = tid;
  if (tid == orbit_base::kAllProcessThreadsTid) {
    name_ = absl::StrFormat("%s\n(all threads)", app_->GetCaptureData().process_name());
  } else {
    name_ = absl::StrFormat("%s\n[%d]", app_->GetCaptureData().GetThreadName(tid_), tid_);
  }
}

void SamplingReportDataView::DoFilter() {
  std::vector<uint64_t> indices;

  std::vector<std::string> tokens = absl::StrSplit(absl::AsciiStrToLower(filter_), ' ');

  for (size_t i = 0; i < functions_.size(); ++i) {
    SampledFunction& func = functions_[i];
    std::string name = absl::AsciiStrToLower(func.name);
    std::string module_name =
        absl::AsciiStrToLower(std::filesystem::path(func.module_path).filename().string());

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

  indices_ = std::move(indices);
}

const SampledFunction& SamplingReportDataView::GetSampledFunction(unsigned int row) const {
  return functions_[indices_[row]];
}

SampledFunction& SamplingReportDataView::GetSampledFunction(unsigned int row) {
  return functions_[indices_[row]];
}
